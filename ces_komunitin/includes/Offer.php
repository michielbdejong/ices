<?php

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;

class Offer {
  public $id;

  // Attributes
  public $code;
}

class OfferSchema extends BaseSchema {

  public function getType(): string {
    return 'offers';
  }

  public function getId($offer): ?string {
    assert($offer instanceof Offer);
    return (string) $offer->id;
  }

  public function getAttributes($offer, ContextInterface $context): iterable {
    assert($offer instanceof Offer);
    $attributes = [
      'code' => $offer->code,
    ];
    return $attributes;
  }

  public function getRelationships($offer, ContextInterface $context): iterable {
    assert($offer instanceof Offer);
    return [];
  }
}
