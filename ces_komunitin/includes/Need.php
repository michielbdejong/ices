<?php

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;

class Need {
  public $id;

  // Attributes
  public $code;
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
    ];
    return $attributes;
  }

  public function getRelationships($need, ContextInterface $context): iterable {
    assert($need instanceof Need);
    return [];
  }
}
