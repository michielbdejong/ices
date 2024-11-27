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

  function __construct($user, Group $group = NULL) {
    $this->id = ces_komunitin_api_social_get_uuid(ResourceTypes::USER, $user->uid);
    $this->email = $user->mail;
    if (!empty($group)) {
      $bank = new CesBank();
      $accounts = $bank->getUserAccounts($user->uid);
      $this->members = [];
      foreach($accounts as $account) {
        $account['user'] = $user;
        $this->members[] = new Member($account, $group);
      }
    } else {
      $this->members = NULL;
    }

    $this->settings = new UserSettings($user);
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
    $relationships = [];
    if (!empty($user->members)) {
      $relationships['members'] = [
        self::RELATIONSHIP_DATA => $user->members,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ];
    }
    if (!empty($user->settings)) {
      $relationships['settings'] = [
        self::RELATIONSHIP_DATA => $user->settings,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ];
    }

    return $relationships;
  }
}
