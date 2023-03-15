<?php
require_once dirname(__FILE__) . '/SchemaUtils.php';

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;

class Event {
  public $id;

  // Attributes
  public $name;
  public $source;
  public $time;
  public $code;

  // Relationships
  public $transfer;

  const TRANSACTION_COMMITTED = 'TransactionCommitted';

  function __construct($event, ExternalTransfer $transfer) {
    global $base_url;
    $this->id = isset($event['id']) ? $event['id'] : null;
    $this->name = $event['name'];
    $this->source = $base_url;
    $this->time = SchemaUtils::encodeDate(time());
    $this->code = $event['code'];
    $this->transfer = $transfer;
  }
}

class EventSchema extends BaseSchema {
  public function getType(): string {
    return 'transfers';
  }
  public function getId($event): ?string {
    assert($event instanceof Event);
    return (string) $event->id;
  }
  public function getAttributes($event, ContextInterface $context): iterable
  {
    assert($event instanceof Event);
    $attributes = [
      'name'  => $event->name,
      'source' => $event->source,
      'time'  => $event->time,
      'code' => $event->code,
    ];
    return $attributes;
  }
  public function getRelationships($event, ContextInterface $context): iterable
  {
    assert($event instanceof Event);
    return [
      'transfer' => [
        self::RELATIONSHIP_DATA => $event->transfer,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ],
    ];
  }
}
