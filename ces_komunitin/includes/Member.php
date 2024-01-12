<?php

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;
use Neomerx\JsonApi\Schema\Identifier;
use Neomerx\JsonApi\Schema\Link;
use Neomerx\JsonApi\Contracts\Schema\LinkInterface;


class Member {
  const TYPE_PERSONAL = 'personal';
  const TYPE_BUSINESS = 'business';
  const TYPE_PUBLIC = 'public';


  public $id;

  // Attributes
  public $code;
  public $access;
  public $name;
  public $type; //"personal" | "business" | "public"
  public $description;
  public $image;
  public $address;
  public $location;

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
    // Only group memebrs can access member details.
    $this->access = "group";
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

    function check_field($entity, $field, $def = null) {
      if (isset($entity->{$field}) && !empty($entity->{$field}[LANGUAGE_NONE][0]['value'])) {
        return $entity->{$field}[LANGUAGE_NONE][0]['value'];
      }
      else {
        return $def;
      }
    }

    $this->image = $user->picture ? file_create_url($user->picture->uri) : null;

    $exchange = $group->exchange;

    $this->address = [
      "streetAddress" => check_field($user, 'ces_address'),
      "addressLocality" => check_field($user, 'ces_town'),
      "postalCode" => check_field($user, 'ces_postcode'),
      "addressRegion" => check_field($user, 'ces_region', $exchange['region']),
      "addressCountry" => check_field($user, 'ces_country', $exchange['country']),
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
      'name' => check_field($user, 'ces_town'),
      'type' => 'Point',
      'coordinates' => [$lng, $lat]
    ];

    $this->description = check_field($user, 'ces_description');

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
    $accountHref = ces_komunitin_api_get_base_url() . '/accounting/' . $member->group->code . '/accounts/' . $member->account_code;
    $needsHref = ces_komunitin_api_get_base_url() . '/social/' . $member->group->code . '/needs?filter[member]=' . $member->id;
    $offersHref = ces_komunitin_api_get_base_url() . '/social/' . $member->group->code . '/offers?filter[member]=' . $member->id;
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
