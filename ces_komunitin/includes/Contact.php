<?php
require_once dirname(__FILE__) . '/SchemaUtils.php';

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;

class Contact {

  public const TYPE_EMAIL = "email";
  public const TYPE_PHONE = "phone";
  public const TYPE_WEBSITE = "website";

  public $groupCode;

  public $id;

  public $type;
  public $name;

  public $created;
  public $updated;

  /**
   * @param $user Drupal user object.
   */
  public function __construct($user, $type, $name, $groupCode) {
    $this->groupCode = $groupCode;
    $this->name = $name;
    $this->type = $type;
    $this->id = ces_komunitin_api_social_get_uuid($type, $user->uid);
    $this->created = SchemaUtils::encodeDate($user->created);
    $this->updated = SchemaUtils::encodeDate($user->created);
  }
}

class ContactSchema extends BaseSchema {
  public function getType(): string {
    return 'contacts';
  }

  public function getId($contact): ?string {
    assert($contact instanceof Contact);
    return (string) $contact->id;
  }

  public function getAttributes($contact, ContextInterface $context): iterable {
    assert($contact instanceof Contact);
    $attributes = [
      'type' => $contact->type,
      'name' => $contact->name,
      'created' => $contact->created,
      'updated' => $contact->updated
    ];
    return $attributes;
  }

  public function getRelationships($contact, ContextInterface $context): iterable {
    assert($contact instanceof Contact);
    return [];
  }

  protected function getSelfSubUrl($resource): string {
    return '/' . $resource->groupCode . $this->getResourcesSubUrl() . '/' . $resource->id;
  }
}
