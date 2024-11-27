<?php
require_once dirname(__FILE__) . '/SchemaUtils.php';

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;

class Transfer {
  /**
  * @var string
  */
  public $id;
  /**
  * @var Account
  */
  public $payer;
  /**
  * @var Account
  */
  public $payee;
  /**
   * @var Currency
   */
  public $currency;
  /**
  * @var integer
  */
  public $amount;
  /**
  * @var string
  */
  public $meta;
  /**
   * @var string
   */
  public $state;
  /**
   * @var string
   */
  public $created;
  /**
   * @var string
   */
  public $updated;

  // Expires in not supported in IntegralCES.
  // public $expires;

  // Komunitin Accounting protocol Transfer states.
  public const STATE_NEW = 'new';
  public const STATE_PENDING = 'pending';
  public const STATE_ACCEPTED = 'accepted';
  public const STATE_COMMITTED = 'committed';
  public const STATE_REJECTED = 'rejected';
  public const STATE_DELETED = 'deleted';

  public const STATES = [self::STATE_NEW, self::STATE_PENDING, self::STATE_ACCEPTED, self::STATE_COMMITTED, self::STATE_REJECTED, self::STATE_DELETED];
  /**
   * Error is an internal state, not to be returned by API.
   */
  private const STATE_ERROR = 'error';

  // IntegralCES state to Komunitin state
  private static $states = array(
    0 => self::STATE_NEW, // Not performed
    1 => self::STATE_PENDING, // Awaiting acceptance
    2 => self::STATE_ACCEPTED, // Accepted
    3 => self::STATE_COMMITTED, // Committed
    4 => self::STATE_COMMITTED, // Archived
    5 => self::STATE_REJECTED, // Rejected
    6 => self::STATE_DELETED, // Discarded
    7 => self::STATE_COMMITTED, // Revoke triggered
    8 => self::STATE_REJECTED, // Revoke accepted
    9 => self::STATE_COMMITTED, // Revoke rejected
    10 => self::STATE_REJECTED, // Revoked
    11 => self::STATE_ERROR // Error
  );


  /**
   * Create a Transfer object.
   *
   * @param array $transaction The ces_bank transaction record.
   */
  function __construct($transaction, $exchange, Currency $currency) {
    $bank = new CesBank();
    $this->id = $transaction['uuid'];

    // Tempos
    $this->created = SchemaUtils::encodeDate($transaction['created']);
    $this->updated = SchemaUtils::encodeDate($transaction['modified']);

    // State
    $this->state = self::$states[$bank->getTransactionState($transaction)];

    // Amount
    $scale = $currency->scale;
    $this->amount = round(pow(10, $scale) * $bank->getTransactionAmount($transaction, $exchange));

    // Meta
    $this->meta = $transaction['concept'];

    // Payer
    $payer = $bank->getTransactionFromAccount($transaction);
    $this->payer = new Account($payer, $currency);

    // Payee
    $payee = $bank->getTransactionToAccount($transaction);
    $this->payee = new Account($payee, $currency);

    // Currency
    $this->currency = $currency;
  }

}

class TransferSchema extends BaseSchema {
  public function getType(): string {
    return 'transfers';
  }

  public function getId($transfer): ?string {
    assert($transfer instanceof Transfer);
    return (string) $transfer->id;
  }

  /**
   * @param Transfer $transfer
   */
  public function getAttributes($transfer, ContextInterface $context): iterable
  {
    assert($transfer instanceof Transfer);
    $attributes = [
      'amount'  => $transfer->amount,
      'meta' => $transfer->meta,
      'state'  => $transfer->state,
      'created' => $transfer->created,
      'updated' => $transfer->updated,
    ];
    return $attributes;
  }

  /**
   * @param Transfer $transfer
   */
  public function getRelationships($transfer, ContextInterface $context): iterable
  {
    assert($transfer instanceof Transfer);
    return [
      'payer' => [
        self::RELATIONSHIP_DATA => $transfer->payer,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ],
      'payee' => [
        self::RELATIONSHIP_DATA => $transfer->payee,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ],
      'currency' =>  [
        self::RELATIONSHIP_DATA => $transfer->currency,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ]
    ];
  }

  protected function getSelfSubUrl($resource): string
  {
    return '/' . $resource->currency->code . $this->getResourcesSubUrl() . '/' . $resource->id;;
  }
}
