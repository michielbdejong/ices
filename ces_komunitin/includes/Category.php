<?php

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;

class Category {
  public $id;

  // Attributes
  public $code;
  public $name;
  public $cpa;
  public $description;
  public $icon;
  public $access;
  public $created;
  public $updated;

  // Relationships
  public $group;
  public $offersCount;
  public $needsCount;

  public function __construct($category, Group $group) {
    $this->id = ces_komunitin_api_social_get_uuid(ResourceTypes::CATEGORY, $category->id);
    $this->code = $category->code;
    $this->name = $category->title;
    $this->cpa = null;
    $this->description = $category->description;;
    $this->icon = $category->icon;
    $this->access = 'group';

    $this->group = $group;

    $this->created = $this->group->created;
    $this->updated = $this->created;

    $this->offersCount = ces_komunitin_api_social_category_offers_count($category);
    $this->needsCount = ces_komunitin_api_social_category_needs_count($category);
  }
}

class CategorySchema extends BaseSchema {

  public function getType(): string {
    return 'categories';
  }

  public function getId($category): ?string {
    assert($category instanceof Category);
    return (string) $category->id;
  }

  public function getAttributes($category, ContextInterface $context): iterable {
    assert($category instanceof Category);
    $attributes = [
      'code' => $category->code,
      'name' => $category->name,
      'cpa' => $category->cpa,
      'description' => $category->description,
      'icon' => $category->icon,
      'access' => $category->access,
      'created' => $category->created,
      'updated' => $category->updated,
    ];
    return $attributes;
  }

  public function getRelationships($category, ContextInterface $context): iterable {
    assert($category instanceof Category);
    return [
      'group' => [
        self::RELATIONSHIP_DATA => $category->group,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ],
      'offers' => [
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => true,
        self::RELATIONSHIP_META => ['count' => $category->offersCount]
      ],
      'needs' => [
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => true,
        self::RELATIONSHIP_META => ['count' => $category->needsCount]
      ]
    ];
  }
}
