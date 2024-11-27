<?php

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;
use League\HTMLToMarkdown\HtmlConverter;

/**
 * Group settings.
 */
class GroupSettings {
  public $id;
  public $settings;
  public $code;

  function __construct($exchange) {
    $this->id = ces_komunitin_api_social_get_uuid(ResourceTypes::GROUP_SETTINGS, $exchange['id']);

    // Terms HTML to Markdown.
    if (!empty($exchange['data']['terms_text'])) {
      $format = $exchange['data']['terms_text']['format'];
      if ($format == 'plain_text') {
        $markdown = $exchange['data']['terms_text']['value'];
      } else {
        $html = check_markup($exchange['data']['terms_text']['value'], $exchange['data']['terms_text']['format']);
        $converter = new HtmlConverter();
        $markdown = $converter->convert($html);
      }
    } else {
      $markdown = null;
    }

    $this->settings = [
      "requireAdminApproval" => true,
      "requireAcceptTerms" => !empty($exchange['data']['require_terms']),
      "terms" => $markdown,
      "minOffers" => isset($exchange['data']['registration_offers']) ? $exchange['data']['registration_offers'] : 0,
      "minNeeds" => isset($exchange['data']['registration_wants']) ? $exchange['data']['registration_wants'] : 0,
      "allowAnonymousMemberList" => !empty($exchange['data']['komunitin_allow_anonymous_member_list']),
    ];

    $this->code = $exchange['code'];
  }
}

class GroupSettingsSchema extends BaseSchema {

  public function getType(): string {
    return 'group-settings';
  }

  public function getId($settings): ?string {
    assert($settings instanceof GroupSettings);
    return (string) $settings->id;
  }

  public function getAttributes($settings, ContextInterface $context): iterable {
    assert($settings instanceof GroupSettings);
    return $settings->settings;
  }

  public function getRelationships($settings, ContextInterface $context): iterable {
    assert($settings instanceof GroupSettings);
    return [];
  }

  protected function getSelfSubUrl($settings): string
  {
    return '/' . $settings->code . '/group-settings';
  }
}
