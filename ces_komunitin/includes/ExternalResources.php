<?php

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Contracts\Schema\LinkInterface;
use Neomerx\JsonApi\Schema\BaseSchema;
use Neomerx\JsonApi\Schema\Link;


abstract class ExternalResource {
  public $type;
  public $id;
  public $href;

  function __construct($id, $type, $href) {
    $this->id = $id;
    $this->type = $type;
    $this->href = $href;
  }
}

abstract class ExternalResourceSchema extends BaseSchema {

  public function getId($resource): ?string {
    return (string) $resource->id;
  }

  public function getAttributes($resource, ContextInterface $context): iterable {
    return [];
  }

  public function getRelationships($resource, ContextInterface $context): iterable {
    return [];
  }
  public function hasIdentifierMeta($resource) : bool {
    return TRUE;
  }
  public function getIdentifierMeta($resource) {
    return $this->getResourceMeta($resource);
  }
  public function hasResourceMeta($resource): bool {
    return TRUE;
  }
  public function getResourceMeta($resource) {
    return [
      'external' => TRUE,
      'href' => $resource->href
    ];
  }
  public function getSelfLink($resource): LinkInterface {
    return new Link(false, $resource->href, false);
  }
}

class ExternalAccount extends ExternalResource {};
class ExternalAccountSchema extends ExternalResourceSchema {
  public function getType(): string {
    return "accounts";
  }
};

class ExternalCurrency extends ExternalResource {};
class ExternalCurrencySchema extends ExternalResourceSchema {
  public function getType(): string
  {
    return "currencies";
  }
};

class ExternalTransfer extends ExternalResource{};
class ExternalTransferSchema extends ExternalResourceSchema {
  public function getType(): string {
    return "transfers";
  }

}
