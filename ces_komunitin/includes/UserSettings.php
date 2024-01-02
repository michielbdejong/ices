<?php

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;

/**
 * Settings associated to the User. It includes the language and the
 * notification preferences.
 */
class UserSettings {
  public $id;
  public $settings;

  function __construct($user, $exchange) {
    $this->id = ces_komunitin_api_social_get_uuid(ResourceTypes::USER_SETTINGS, $user->uid);
    module_load_include('module', 'ces_user');
    $this->settings = ces_user_get_settings($user);
  }
}

class UserSettingsSchema extends BaseSchema {

  public function getType(): string {
    return 'user-settings';
  }

  public function getId($settings): ?string {
    assert($settings instanceof UserSettings);
    return (string) $settings->id;
  }

  public function getAttributes($settings, ContextInterface $context): iterable {
    assert($settings instanceof UserSettings);
    return $settings->settings;
  }

  public function getRelationships($settings, ContextInterface $context): iterable {
    assert($settings instanceof UserSettings);
    return [];
  }
}

