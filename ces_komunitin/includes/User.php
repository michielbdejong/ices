<?php

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;


/**
 * User does not carry a lot of information. Is just the representation of a
 * login entity, immediately related to a "member" which is the real object.
 */
class User {
  public $id;

  public $email;

  // Relationships
  public $members;
  public $settings;

  function __construct($user, $exchange) {
    $this->id = ces_komunitin_api_social_get_uuid(ResourceTypes::USER, $user->uid);
    $this->email = $user->mail;

    $bank = new CesBank();
    $accounts = $bank->getUserAccounts($user->uid);
    $this->members = [];
    $group = new Group($exchange);
    foreach($accounts as $account) {
      $account['user'] = $user;
      $this->members[] = new Member($account, $group);
    }
    $this->settings = new UserSettings($user, $exchange);
  }
}

class UserSchema extends BaseSchema
{

  public function getType(): string {
    return 'users';
  }

  public function getId($user): ?string {
    assert($user instanceof User);
    return (string) $user->id;
  }

  public function getAttributes($user, ContextInterface $context): iterable {
    assert($user instanceof User);
    $attributes = [
      "email" => $user->email
    ];
    return $attributes;
  }

  public function getRelationships($user, ContextInterface $context): iterable {
    assert($user instanceof User);
    return [
      'members' => [
        self::RELATIONSHIP_DATA => $user->members,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ],
      'settings' => [
        self::RELATIONSHIP_DATA => $user->settings,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ]
    ];
  }
}
