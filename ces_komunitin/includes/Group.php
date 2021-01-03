<?php
require_once dirname(__FILE__) . '/SchemaUtils.php';

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;
use Neomerx\JsonApi\Schema\Identifier;
use Neomerx\JsonApi\Schema\Link;
use Neomerx\JsonApi\Contracts\Schema\LinkInterface;

use OAuth2\Response;


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


  function __construct($exchange)
  {
    $this->id = ces_komunitin_api_social_get_uuid(ResourceTypes::GROUP, $exchange['id']);

    $this->code = $exchange['code'];
    $this->name = $exchange['name'];

    $this->website = $exchange['website'];
    $this->access = 'public';

    $this->created = SchemaUtils::encodeDate($exchange['created']);
    $this->updated = SchemaUtils::encodeDate($exchange['modified']);

    // Provide at least an email contact: this is the email of the exchange
    // admin user.
    $admin = user_load($exchange['admin']);
    $this->contacts = [new Contact($admin, Contact::TYPE_EMAIL, $this->code)];

    // Fields not yet provided by IntegralCES!
    // Todo: implement these fields: image, description and location for an
    // exchange, and also other means of contact beyond email!
    $this->description = '';
    $this->image = null;
    $this->location = [
      'name' => $exchange['town'],
      'type' => 'Point',
      'coordinates' => [0, 0]
    ];

    // Relationships.
    $this->currency_id = $exchange['uuid_currency'];
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
    return [
      'currency' => [
        self::RELATIONSHIP_DATA => new Identifier($group->currency_id, 'currencies'),
        self::RELATIONSHIP_LINKS_SELF => false,
        // Override default related link
        self::RELATIONSHIP_LINKS => [
          LinkInterface::RELATED => new Link(false, ces_komunitin_api_get_base_url() . '/accounting/' . $group->code . '/currency', false)
        ]
      ],
      'contacts' => [
        self::RELATIONSHIP_DATA => $group->contacts,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ]
    ];
  }
  /**
   * Override default self URL.
   */
  protected function getSelfSubUrl($resource): string {
    return '/' . $resource->code;
  }
}
