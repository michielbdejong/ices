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
  public $data;

  // Relationships
  public $user;

  const TRANSFER_COMMITTED = 'TransferCommitted';
  const TRANSFER_PENDING = 'TransferPending';
  const TRANSFER_REJECTED = 'TransferRejected';
  const NEED_PUBLISHED = 'NeedPublished';
  const NEED_EXPIRED = 'NeedExpired';
  const OFFER_PUBLISHED = 'OfferPublished';
  const OFFER_EXPIRED = 'OfferExpired';
  const MEMBER_JOINED = 'MemberJoined';
  const MEMBER_REQUESTED = 'MemberRequested';
  const GROUP_ACTIVATED = 'GroupActivated';
  const GROUP_REQUESTED = 'GroupRequested';

  const SOCIAL_EVENTS = [
    self::NEED_PUBLISHED,
    self::NEED_EXPIRED,
    self::OFFER_PUBLISHED,
    self::OFFER_EXPIRED,
    self::MEMBER_JOINED,
    self::MEMBER_REQUESTED,
    self::GROUP_ACTIVATED,
    self::GROUP_REQUESTED
  ];
  const ACCOUNTING_EVENTS = [
    self::TRANSFER_COMMITTED,
    self::TRANSFER_PENDING,
    self::TRANSFER_REJECTED
  ];

  /**
   * @deprecated Use Event::create instead.
   *
   * @param $event The event data as an string-indexed associative array.
   * @param $user The ExternalUser object.
   */
  function __construct($event, ExternalUser $user) {
    global $base_url;
    $this->id = isset($event['id']) ? $event['id'] : null;
    $this->name = $event['name'];
    
    // Use the API base url in the source field.
    if (in_array($event['name'], self::SOCIAL_EVENTS)) {
      $this->source = $base_url . '/ces/api/social';
    } else if (in_array($event['name'], self::ACCOUNTING_EVENTS)) {
      $this->source = $base_url . '/ces/api/accounting';
    } else {
      $this->source = $base_url;
    }
    
    $this->time = SchemaUtils::encodeDate(time());
    $this->code = $event['code'];
    $this->user = $user;
    $this->data = $event['data'];
  }

  /**
   * Create an Event object.
   *
   * @param $name The event name.
   * @param $code The exhange code.
   * @param $data The event data as an string-indexed associative array.
   */
  static function create($name, $code, $data) {
    global $user;
    $userId = ces_komunitin_api_social_get_uuid(ResourceTypes::USER, $user->uid);
    $urlPrefix = ces_komunitin_api_get_social_api_url();
    $userHref = $urlPrefix . '/users/' . $userId;
    $externalUser = new ExternalUser($userId, 'users', $userHref);

    $event = [
      'name' => $name,
      'code' => $code,
      'data' => $data
    ];

    return new Event($event, $externalUser);
  }
}

class EventSchema extends BaseSchema {
  public function getType(): string {
    return 'events';
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
      'data' => $event->data
    ];
    return $attributes;
  }
  public function getRelationships($event, ContextInterface $context): iterable
  {
    assert($event instanceof Event);
    $relationships = [
      'user' => [
        self::RELATIONSHIP_DATA => $event->user,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ]
    ];
    return $relationships;
  }
}
