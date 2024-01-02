<?php
require_once dirname(__FILE__) . '/Need.php';

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;

class Offer extends Need {
  // Attributes in offers
  public $name;
  public $price;

  public function __construct($resource, Member $member, Group $group, Category $category) {
    parent::__construct($resource, $member, $group, $category);
    $this->id = ces_komunitin_api_social_get_uuid(ResourceTypes::OFFER, $resource->id);
    $this->name = $resource->title;
    $this->price = $resource->ces_offer_rate[LANGUAGE_NONE][0]['value'];
  }
}

class OfferSchema extends NeedSchema {

  public function getType(): string {
    return 'offers';
  }

  public function getAttributes($offer, ContextInterface $context): iterable {
    assert($offer instanceof Offer);
    $attributes = parent::getAttributes($offer, $context);
    $attributes['name'] = $offer->name;
    $attributes['price'] = $offer->price;

    return $attributes;
  }

}
