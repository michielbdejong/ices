<?php
require_once dirname(__FILE__) . '/SchemaUtils.php';

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;
use Neomerx\JsonApi\Schema\Link;
use Neomerx\JsonApi\Contracts\Schema\LinkInterface;

class Group {
  public $id;

  // Attributes
  public $code;
  public $name;
  public $description;
  public $image;
  public $website;
  public $access;
  public $location;

  public $created;
  public $updated;

  // Relationships
  public $currency_id;
  public $contacts;
  public $categories;

  public $membersCount;
  public $needsCount;
  public $offersCount;

  // Helper
  public $exchange;


  function __construct($exchange)
  {
    $this->id = ces_komunitin_api_social_get_uuid(ResourceTypes::GROUP, $exchange['id']);

    $this->code = $exchange['code'];
    $this->name = $exchange['name'];

    $this->website = $exchange['website'];
    $this->access = 'public';

    $this->created = SchemaUtils::encodeDate($exchange['created']);
    $this->updated = SchemaUtils::encodeDate($exchange['modified']);

    // Fields not yet provided by IntegralCES!
    // Todo: implement these fields: image, description and location for an
    // exchange, and also other means of contact beyond email!
    $this->description = '';
    $this->image = null;
    $this->location = [
      'name' => $exchange['town'],
      'type' => 'Point',
      'coordinates' => [$exchange['lng'], $exchange['lat']]
    ];

    // Relationships.
    // Provide at least an email contact: this is the email of the exchange
    // admin user.
    $admin = user_load($exchange['admin']);
    $this->contacts = [new Contact($admin, Contact::TYPE_EMAIL, $admin->mail, $this->code)];

    // Load categories for this group.
    $categories = ces_komunitin_api_social_categories_load_collection($exchange, null, null);
    $this->categories = array_map(function($category) {
      return new Category($category, $this);
    }, $categories);

    $this->currency_id = $exchange['uuid_currency'];
    $this->membersCount = ces_komunitin_api_social_members_count($exchange);
    $this->needsCount = ces_komunitin_api_social_needs_count($exchange);
    $this->offersCount = ces_komunitin_api_social_offers_count($exchange);

    $this->exchange = $exchange;
  }
}

class GroupSchema extends BaseSchema {

  public function getType(): string {
    return 'groups';
  }

  public function getId($group): ?string {
    assert($group instanceof Group);
    return (string) $group->id;
  }

  public function getAttributes($group, ContextInterface $context): iterable {
    assert($group instanceof Group);
    $attributes = [
      'code' => $group->code,
      'name' => $group->name,
      'description' => $group->description,
      'image' => $group->image,
      'website' => $group->website,
      'access' => $group->access,
      'location' => $group->location,
      'created' => $group->created,
      'updated' => $group->updated,
    ];
    return $attributes;
  }

  public function getRelationships($group, ContextInterface $context): iterable {
    assert($group instanceof Group);
    $currencyHref = ces_komunitin_api_get_base_url() . '/accounting/' . $group->code . '/currency';
    return [
      'currency' => [
        self::RELATIONSHIP_DATA => new ExternalCurrency($group->currency_id, 'currencies', $currencyHref),
        self::RELATIONSHIP_LINKS_SELF => false,
        // Override default related link
        self::RELATIONSHIP_LINKS => [
          LinkInterface::RELATED => new Link(false, $currencyHref, false)
        ]
      ],
      'contacts' => [
        self::RELATIONSHIP_DATA => $group->contacts,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ],
      'members' => [
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => true,
        self::RELATIONSHIP_META => ['count' => $group->membersCount]
      ],
      'categories' => [
        self::RELATIONSHIP_DATA => $group->categories,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false,
        self::RELATIONSHIP_META => ['count' => count($group->categories)]
      ],
      'offers' => [
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => true,
        self::RELATIONSHIP_META => ['count' => $group->offersCount]
      ],
      'needs' => [
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => true,
        self::RELATIONSHIP_META => ['count' => $group->needsCount]
      ],
    ];
  }
  /**
   * Override default self URL.
   */
  protected function getSelfSubUrl($resource): string {
    return '/' . $resource->code;
  }
}
