<?php

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;
use League\HTMLToMarkdown\HtmlConverter;

class Need {
  public $groupCode;
  public $id;

  // Attributes
  public $code;
  public $content;
  public $images;
  public $access;
  public $state;
  public $expires;
  public $created;
  public $updated;

  // Realtionships
  public $category;
  public $member;

  public function __construct($resource, Member $member, Group $group, Category $category) {
    $this->groupCode = $group->code;
    $this->id = ces_komunitin_api_social_get_uuid(ResourceTypes::NEED, $resource->id);
    // Attributes
    $this->code = $resource->code;
    $converter = new HtmlConverter();
    $markdown = $converter->convert($resource->body);
    $this->content = $markdown;
    $this->images = [];
    if ($resource->image) {
      $images = explode(',', $resource->image);
      foreach ($images as $image) {
        $file = file_load($image);
        $this->images[] = file_create_url($file->uri);
      }
    }

    $this->access = 'group';
    $states = ['hidden', 'published'];
    $this->state = $states[$resource->state];
    $this->expires = SchemaUtils::encodeDate($resource->expire);
    $this->created = SchemaUtils::encodeDate($resource->created);
    $this->updated = SchemaUtils::encodeDate($resource->modified);

    // Relationships
    $this->category = $category;
    $this->member = $member;
  }
}

class NeedSchema extends BaseSchema {

  public function getType(): string {
    return 'needs';
  }

  public function getId($need): ?string {
    assert($need instanceof Need);
    return (string) $need->id;
  }

  public function getAttributes($need, ContextInterface $context): iterable {
    assert($need instanceof Need);
    $attributes = [
      'code' => $need->code,
      'content' => $need->content,
      'images' => $need->images,
      'access' => $need->access,
      'state' => $need->state,
      'expires' => $need->expires,
      'created' => $need->created,
      'updated' => $need->updated
    ];
    return $attributes;
  }

  public function getRelationships($need, ContextInterface $context): iterable {
    assert($need instanceof Need);
    return [
      'category' => [
        self::RELATIONSHIP_DATA => $need->category,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ],
      'member' => [
        self::RELATIONSHIP_DATA => $need->member,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ],
    ];
  }

  protected function getSelfSubUrl($resource): string {
    assert($resource instanceof Need);
    return '/' . $resource->groupCode . '/'. $this->getType() .'/' . $resource->code;
  }
}
