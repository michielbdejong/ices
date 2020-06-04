<?php

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;

class Transfer {
  public $baseUrl;

  public $payer;
  public $localPayer;
  public $payee;
  public $localPayee;
  public $amount;
  public $meta;

  function getPayerUrl() {
    return $this->accountUrl($this->payer);
  }

  function getPayeeUrl() {
    return $this->accountUrl($this->payee);
  }

  function getLocalPayerUrl() {
    return $this->accountUrl($this->localPayer);
  }

  function getLocalPayeeUrl() {
    return $this->accountUrl($this->localPayee);
  }

  private function accountUrl($name) {
    return ($name != null) ? ($this->baseUrl . '/' .  $name) : null;
  }
}


class Transaction {
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

  public $id;
  // Attributes.
  /**
   * @var Transfer
   */
  public $transfer;
  public $state;
  public $created;
  public $updated;

  // Expires in not supported in IntegralCES.
  // public $expires;

  // Relationships.

  // Currency still not implemented.
  // public $currency;

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
   * Create a Transaction object.
   */
  function __construct($base_url, $transaction, $exchange) {
    $this->id = $transaction['uuid'];

    // Tempos
    $this->created = $this->encodeDate($transaction['created']);
    $this->updated = $this->encodeDate($transaction['modified']);

    // State
    $bank = new CesBank();
    $this->state = self::$states[$bank->getTransactionState($transaction)];

    // Transfer
    $this->transfer = new Transfer();
    $this->transfer->baseUrl = $base_url;

    $payer = $bank->getTransactionFromAccount($transaction);
    $this->transfer->payer = $payer['name'];

    if ($payer['id'] != $transaction['fromaccount']) {
      $localPayer = $bank->getAccount($transaction['fromaccount']);
      $this->transfer->localPayer = $localPayer['name'];
    }

    $payee = $bank->getTransactionToAccount($transaction);
    $this->transfer->payee = $payee['name'];

    if ($payee['id'] != $transaction['toaccount']) {
      $localPayee = $bank->getAccount($transaction['toaccount']);
      $this->transfer->localPayee = $localPayee['name'];
    }

    $decimals = $exchange['currencyscale'];
    $this->transfer->amount = round(pow(10, $decimals) * $bank->getTransactionAmount($transaction, $exchange));

    $this->transfer->meta = $transaction['concept'];
  }

  private function encodeDate($timestamp) {
    return DateTime::createFromFormat('U', $timestamp)->format(DateTime::ATOM);
  }
}

class TransactionSchema extends BaseSchema{

  public function getType(): string {
    return 'transactions';
  }

  public function getId($transaction): ?string {
    assert($transaction instanceof Transaction);
    return (string) $transaction->id;
  }

  /**
   * @param Transaction $transaction
   */
  public function getAttributes($transaction, ContextInterface $context): iterable {
    assert($transaction instanceof Transaction);
    $attributes = [
      'transfers' => array(
        array(
          'payer' => $transaction->transfer->getPayerUrl(),
          'payee' => $transaction->transfer->getPayeeUrl(),
          'amount' => $transaction->transfer->amount,
          'meta' => $transaction->transfer->meta
        )
      ),
      'state'  => $transaction->state,
      'created' => $transaction->created,
      'updated' => $transaction->updated,
    ];
    $localPayer = $transaction->transfer->getLocalPayerUrl();
    if ($localPayer) {
      $attributes['transfers'][0]['localPayer'] = $localPayer;
    }
    $localPayee = $transaction->transfer->getLocalPayeeUrl();
    if ($localPayee) {
      $attributes['transfers'][0]['localPayee'] = $localPayee;
    }
    return $attributes;
  }

  public function getRelationships($transaction, ContextInterface $context): iterable {
    assert($transaction instanceof Transaction);
    return [];
  }
}

