<?php

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;
use League\HTMLToMarkdown\HtmlConverter;

/**
 * Group signup settings.
 */
class SignupSettings {
  public $id;
  public $settings;
  public $code;

  function __construct($exchange) {
    $this->id = ces_komunitin_api_social_get_uuid(ResourceTypes::SIGNUP_SETTINGS, $exchange['id']);

    // Terms HTML to Markdown.
    if (!empty($exchange['data']['terms_text'])) {
      $html = check_markup($exchange['data']['terms_text']['value'], $exchange['data']['terms_text']['format']);
      $converter = new HtmlConverter();
      $markdown = $converter->convert($html);
    } else {
      $markdown = null;
    }

    $this->settings = [
      "requireAdminApproval" => true,
      "requireAcceptTerms" => !empty($exchange['data']['require_terms']),
      "terms" => $markdown,
      "minOffers" => isset($exchange['data']['registration_offers']) ? $exchange['data']['registration_offers'] : 0,
      "minNeeds" => isset($exchange['data']['registration_offers']) ? $exchange['data']['registration_wants'] : 0
    ];

    $this->code = $exchange['code'];
  }
}

class SignupSettingsSchema extends BaseSchema {

  public function getType(): string {
    return 'signup-settings';
  }

  public function getId($settings): ?string {
    assert($settings instanceof SignupSettings);
    return (string) $settings->id;
  }

  public function getAttributes($settings, ContextInterface $context): iterable {
    assert($settings instanceof SignupSettings);
    return $settings->settings;
  }

  public function getRelationships($settings, ContextInterface $context): iterable {
    assert($settings instanceof SignupSettings);
    return [];
  }

  protected function getSelfSubUrl($settings): string
  {
    return '/' . $settings->code . '/signup-settings';
  }
}
