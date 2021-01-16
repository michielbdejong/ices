<?php

use Neomerx\JsonApi\Contracts\Encoder\EncoderInterface;
use Neomerx\JsonApi\Contracts\Representation\DocumentWriterInterface;
use Neomerx\JsonApi\Contracts\Schema\LinkInterface;
use Neomerx\JsonApi\Factories\Factory;
use Neomerx\JsonApi\Representation\DocumentWriter;

class CustomFactory extends Factory {
  /**
   * @inheritdoc
   */
  public function createDocumentWriter(): DocumentWriterInterface {
    return new CustomDocumentWriter();
  }

  public static function newEncoder(array $schemas = []): EncoderInterface{
    $factory   = new CustomFactory();
    $container = $factory->createSchemaContainer($schemas);
    $encoder   = $factory->createEncoder($container);
    return $encoder;
  }
}
/**
 * Extension of DocumentWriter that allows null links, necessary for the jsonapi
 * profile https://jsonapi.org/profiles/ethanresnick/cursor-pagination/
 */
class CustomDocumentWriter extends DocumentWriter {
  /**
   * @inheritdoc
   */
  protected function getLinksRepresentation(?string $prefix, iterable $links): array
  {
    $result = [];
    foreach ($links as $name => $link) {
      if ($link === null) {
        $result[$name] = null;
      } else {
        // This is the same code found in parent.
        \assert($link instanceof LinkInterface);
        $result[$name] = $link->canBeShownAsString() === true ?
          $link->getStringRepresentation($prefix) : $link->getArrayRepresentation($prefix);
      }
    }

    return $result;
  }
}
