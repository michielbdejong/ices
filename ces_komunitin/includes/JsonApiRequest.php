<?php

use Neomerx\JsonApi\Http\Query\BaseQueryParser;
use Neomerx\JsonApi\Contracts\Http\Query\BaseQueryParserInterface;
use Neomerx\JsonApi\Exceptions\JsonApiException;
use Neomerx\JsonApi\Schema\Error;

class JsonApiRequest {
  /**
   * Uppercase http request method.
   */
  public $method;
  public $path;

  public $includes;
  public $fields;
  public $filters;
  public $sorts;

  public $body;

  /**
   * Build a new JsonApiRequest object.
   *
   * @param $base_path string The path prefix of the Api, as defined in hook_menu.
   */
  function __construct($base_path) {
    $this->method = strtoupper($_SERVER['REQUEST_METHOD']);

    $this->path = mb_substr($_GET['q'], mb_strlen($base_path . '/'));

    $parser = new BaseQueryParser($_GET);
    $this->includes = $parser->getIncludes();
    $this->fields = $parser->getFields();
    $this->sorts = $parser->getSorts();
    $this->filters = $this->getFilters();

    if ($this->method == 'POST' || $this->method == 'PATCH') {
      $this->body = file_get_contents('php://input');
    }
  }

  /**
   * The library we use for JSONApi request parsing does not parse the "filter"
   * parameter, so we do it manually.
   */
  private function getFilters() {
    $parameters = $_GET;
    if (\array_key_exists(BaseQueryParserInterface::PARAM_FILTER, $parameters)) {
      $filters = $parameters[BaseQueryParserInterface::PARAM_FILTER];
      if (!\is_array($filters) || empty($filters)) {
        // Invalid filter query parameter. Throw same type of exception as library.
        $source = [Error::SOURCE_PARAMETER => BaseQueryParserInterface::PARAM_FILTER];
        $error  = new Error(null, null, null, null, null, BaseQueryParser::MSG_ERR_INVALID_PARAMETER, null, $source);
        throw new JsonApiException(new Error($error));
      }
      foreach ($filters as $field => $value) {
        yield $field => explode(',', $value);
      }
    }
  }
}
