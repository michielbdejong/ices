<?php

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;
use Neomerx\JsonApi\Schema\Identifier;
use Neomerx\JsonApi\Schema\Link;
use Neomerx\JsonApi\Contracts\Schema\LinkInterface;

function _ces_komunitin_check_field($entity, $field, $def = null) {
  if (isset($entity->{$field}) && !empty($entity->{$field}[LANGUAGE_NONE][0]['value'])) {
    return $entity->{$field}[LANGUAGE_NONE][0]['value'];
  }
  else {
    return $def;
  }
}

class Member {
  const TYPE_PERSONAL = 'personal';
  const TYPE_BUSINESS = 'business';
  const TYPE_PUBLIC = 'public';

  const STATE_PENDING = 'pending';
  const STATE_ACTIVE = 'active';
  const STATE_SUSPENDED = 'suspended';
  const STATE_DELETED = 'deleted';


  public $id;

  // Attributes
  public $code;
  public $access;
  public $name;
  public $type; // "personal" | "business" | "public"
  public $description;
  public $image;
  public $address;
  public $location;
  public $state; // "pending" | "active" | "suspended" | "deleted"

  public $created;
  public $updated;

  // Relationships
  public $group;
  public $contacts;
  public $account_id;
  public $account_code;

  public $offersCount;
  public $needsCount;

  public function __construct($member, $group) {
    $user = $member['user'];
    $this->id = ces_komunitin_api_social_get_uuid(ResourceTypes::MEMBER, $member['id']);
    // Use drupal username as komunitin user code.
    $this->code = $member['name'];

    $this->access = "group";// This is not really used
    $this->name = ces_user_get_name($user);

    // Member type.
    switch($member['kind']) {
      case CesBankLocalAccount::TYPE_INDIVIDUAL:
      case CesBankLocalAccount::TYPE_SHARED:
        $this->type = self::TYPE_PERSONAL;
        break;
      case CesBankLocalAccount::TYPE_COMPANY:
      case CesBankLocalAccount::TYPE_ORGANIZATION:
        $this->type = self::TYPE_BUSINESS;
        break;
      case CesBankLocalAccount::TYPE_PUBLIC:
      case CesBankLocalAccount::TYPE_VIRTUAL:
        $this->type = self::TYPE_PUBLIC;
    }

    $this->image = $user->picture ? file_create_url($user->picture->uri) : null;

    $exchange = $group->exchange;

    $this->address = [
      "streetAddress" => _ces_komunitin_check_field($user, 'ces_address'),
      "addressLocality" => _ces_komunitin_check_field($user, 'ces_town'),
      "postalCode" => _ces_komunitin_check_field($user, 'ces_postcode'),
      "addressRegion" => _ces_komunitin_check_field($user, 'ces_region', $exchange['region']),
      "addressCountry" => _ces_komunitin_check_field($user, 'ces_country', $exchange['country']),
    ];

    $this->created = SchemaUtils::encodeDate($user->created);
    $this->updated = SchemaUtils::encodeDate($member['modified']);

    $items = field_get_items('user', $user, 'ces_geolocation');
    $lat = 0;
    $lng = 0;
    if (!empty($items)) {
      $item = reset($items);
      $lat = $item['lat'];
      $lng = $item['lng'];
    }

    $this->location = [
      'name' => _ces_komunitin_check_field($user, 'ces_town'),
      'type' => 'Point',
      'coordinates' => [$lng, $lat]
    ];

    $this->description = _ces_komunitin_check_field($user, 'ces_description');

    // State
    switch ($member['state']) {
      case CesBankLocalAccount::STATE_ACTIVE:
        $this->state = self::STATE_ACTIVE;
        break;
      case CesBankLocalAccount::STATE_HIDDEN:
        $this->state = self::STATE_PENDING;
        break;
      case CesBankLocalAccount::STATE_LOCKED:
        $this->state = self::STATE_SUSPENDED;
        break;
      case CesBankLocalAccount::STATE_CLOSED:
      default:
        $this->state = self::STATE_DELETED;
        break;
    }

    // Relationships
    $this->account_id = $member['uuid'];
    $this->account_code = $member['name'];
    $this->group = $group;
    $this->contacts = [];
    // email
    $contacts = ces_user_get_contacts($user);
    foreach ($contacts as $type => $name) {
      $this->contacts[] = new Contact($user, $type, $name, $this->group->code);
    }

    $this->offersCount = ces_komunitin_api_social_account_offers_count($member['id'], $member['exchange']);
    $this->needsCount = ces_komunitin_api_social_account_needs_count($member['id'], $member['exchange']);
  }

}

class MemberSchema extends BaseSchema {

  public function getType(): string {
    return 'members';
  }

  public function getId($member): ?string {
    assert($member instanceof Member);
    return (string) $member->id;
  }

  public function getAttributes($member, ContextInterface $context): iterable {
    assert($member instanceof Member);
    $attributes = [
      'code' => $member->code,
      'name' => $member->name,
      'access' => $member->access,
      'state' => $member->state,
      'type' => $member->type,
      'description' => $member->description,
      'image' => $member->image,
      'address' => $member->address,
      'location' => $member->location,
      'created' => $member->created,
      'updated' => $member->updated,
    ];
    return $attributes;
  }

  public function getRelationships($member, ContextInterface $context): iterable {
    assert($member instanceof Member);
    $accountHref = ces_komunitin_api_get_accounting_api_url($member->group->exchange) . '/' . $member->group->code . '/accounts/' . $member->account_code;
    $needsHref = ces_komunitin_api_get_social_api_url() . '/' . $member->group->code . '/needs?filter[member]=' . $member->id;
    $offersHref = ces_komunitin_api_get_social_api_url() . '/' . $member->group->code . '/offers?filter[member]=' . $member->id;
    return [
      'group' => [
        self::RELATIONSHIP_DATA => $member->group,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ],
      'account' => [
        self::RELATIONSHIP_DATA => new ExternalAccount($member->account_id, 'accounts', $accountHref),
        self::RELATIONSHIP_LINKS_SELF => false,
        // Override default related link
        self::RELATIONSHIP_LINKS => [
          LinkInterface::RELATED => new Link(false, $accountHref, false)
        ],
      ],
      'contacts' => [
        self::RELATIONSHIP_DATA => $member->contacts,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ],
      'needs' => [
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS => [
          LinkInterface::RELATED => new Link(false, $needsHref, false)
        ],
        self::RELATIONSHIP_META => ['count' => $member->needsCount]
      ],
      'offers' => [
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS => [
          LinkInterface::RELATED => new Link(false, $offersHref, false)
        ],
        self::RELATIONSHIP_META => ['count' => $member->offersCount]
      ],
    ];
  }

  protected function getSelfSubUrl($resource): string {
    assert($resource instanceof Member);
    return '/' . $resource->group->code . $this->getResourcesSubUrl() . '/' . $resource->code;
  }
}

class MinimalMember extends Member {}

class MinimalMemberSchema extends MemberSchema {
  public function getAttributes($member, ContextInterface $context): iterable {
    assert($member instanceof Member);
    $attributes = [
      'name' => $member->name,
      'image' => $member->image,
    ];
    return $attributes;
  }

  public function getRelationships($member, ContextInterface $context): iterable {
    assert($member instanceof Member);
    $fullRelationships = parent::getRelationships($member, $context);

    $relationships = [
      'group' => $fullRelationships['group'],
      'account' => $fullRelationships['account'],
    ];

    return $relationships;
  }
}
