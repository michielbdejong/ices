<?php

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;

class Category {
  public $id;

  // Attributes
  public $code;
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
    ];
    return $attributes;
  }

  public function getRelationships($category, ContextInterface $context): iterable {
    assert($category instanceof Category);
    return [];
  }
}
