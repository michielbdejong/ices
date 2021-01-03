<?php

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;
use Neomerx\JsonApi\Schema\Identifier;

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

  public function __construct($user, $exchange) {
    $this->id = ces_komunitin_api_social_get_uuid(ResourceTypes::MEMBER, $user->uid);
    // Use drupal username as komunitin user code.
    $this->code = $user->name;
    // Only group memebrs can access member details.
    $this->access = "group";
    $this->name = ces_user_get_name($user);

    $bank = new CesBank();
    // member account
    // Get the first account from the current exchange
    $accounts = $bank->getUserAccounts($user->uid);
    $account = FALSE;
    foreach ($accounts as $candidate) {
      if ($candidate['exchange'] == $exchange['id']) {
        $account = $candidate;
        break;
      }
    }
    if (!$account) {
      // Member has no account in this exchange. So we say that the member does
      // not exist.
      throw new Exception("Member doesn't have any account in this exchange group");
    }
    // Member type.
    switch($account['kind']) {
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
    $this->address = ces_user_get_full_address($user);
    // Get user town.
    $town = '';
    $items = field_get_items('user', $account, 'ces_town');
    if (!empty($items)) {
      $item = reset($items);
      $town = $item['safe_value'];
    }

    $this->created = SchemaUtils::encodeDate($user->created);
    $this->updated = SchemaUtils::encodeDate($account['modified']);


    // There's no member description nor location.
    $this->description = '';
    $this->location = [
      'name' => $town,
      'type' => 'Point',
      'coordinates' => [0, 0]
    ];

    // Relationships
    $this->account_id = $account['uuid'];
    $this->group = new Group($exchange);
    $this->contacts = [];
    // email
    $this->contacts[] = new Contact($user, Contact::TYPE_EMAIL, $this->group->code);
    if (ces_user_get_main_phone($user)) {
      $this->contacts[] = new Contact($user, Contact::TYPE_PHONE, $this->group->code);
    }
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
    return [
      'group' => [
        self::RELATIONSHIP_DATA => $member->group,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ],
      'account' => [
        self::RELATIONSHIP_DATA => new Identifier($member->account_id, 'accounts'),
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => true
      ],
      'contacts' => [
        self::RELATIONSHIP_DATA => $member->contacts,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ]
    ];
  }

  protected function getSelfSubUrl($resource): string {
    assert($resource instanceof Member);
    return '/' . $resource->group->code . $this->getResourcesSubUrl() . '/' . $resource->code;
  }
}
