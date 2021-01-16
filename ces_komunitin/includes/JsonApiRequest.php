<?php

use Neomerx\JsonApi\Http\Query\BaseQueryParser;
use Neomerx\JsonApi\Contracts\Http\Query\BaseQueryParserInterface;
use Neomerx\JsonApi\Exceptions\JsonApiException;
use Neomerx\JsonApi\Schema\Error;

class JsonApiRequest {
  const DEFAULT_PAGE_SIZE = 20;
  /**
   * Uppercase http request method.
   */
  public $method;
  public $basePath;
  public $path;

  public $includes;
  public $fields;
  public $filters;
  public $sorts;

  public $pageSize;
  public $pageAfter;

  public $body;

  /**
   * Build a new JsonApiRequest object.
   *
   * @param $base_path string The path prefix of the Api, as defined in hook_menu.
   */
  function __construct($base_path) {
    $this->basePath = $base_path;
    $this->method = strtoupper($_SERVER['REQUEST_METHOD']);

    $this->path = mb_substr($_GET['q'], mb_strlen($base_path . '/'));

    $parser = new BaseQueryParser($_GET);
    $this->includes = $parser->getIncludes();
    $this->fields = $parser->getFields();
    $this->sorts = $parser->getSorts();
    $this->filters = $this->getFilters();

    $page = $this->getPage();
    $this->pageSize = $page['size'] ?? self::DEFAULT_PAGE_SIZE;
    $this->pageAfter = $page['after'] ?? null;

    if ($this->method == 'POST' || $this->method == 'PATCH') {
      $this->body = file_get_contents('php://input');
    }
  }

  /**
   * The library we use for JSONApi request parsing does not parse the "filter"
   * parameter, so we do it manually.
   */
  private function getFilters() {
    $filters = $this->getArrayParameter(BaseQueryParserInterface::PARAM_FILTER);
    if ($filters) {
      foreach ($filters as $field => $value) {
        yield $field => explode(',', $value);
      }
    }
  }

  /**
   * Return array with page parameters.
   */
  private function getPage() {
    $page = $this->getArrayParameter(BaseQueryParserInterface::PARAM_PAGE);
    return $page ?? [];
  }

  /**
   * Return parameter value or null.
  */
  private function getArrayParameter($name) {
    $parameters = $_GET;
    if (\array_key_exists($name, $parameters)) {
      $value = $parameters[$name];
      if (!\is_array($value) || empty($value)) {
        // Invalid filter query parameter. Throw same type of exception as library.
        $source = [Error::SOURCE_PARAMETER => $name];
        $error  = new Error(null, null, null, null, null, BaseQueryParser::MSG_ERR_INVALID_PARAMETER, null, $source);
        throw new JsonApiException(new Error($error));
      }
      return $value;
    }
    return null;
  }

  public function getNextUrl() {
    $path = $this->basePath . '/' . $this->path;
    $query = [];
    if ($this->includes) {
      $query['include'] = implode(",", array_map(function($item){
        return implode(".", iterator_to_array($item));
      }, iterator_to_array($this->includes)));
    }
    if ($this->fields) {
      $query['field'] = array_map(function($item) {
        return implode(",", iterator_to_array($item));
      }, iterator_to_array($this->fields));
    }
    if ($this->filters) {
      $query['filter'] = array_map(function($item) {
        return implode(",", $item);
      }, iterator_to_array($this->filters));
    }
    if ($this->sorts) {
      $sorts = [];
      foreach ($this->sorts as $field => $isAsc) {
        $sorts[] = ($isAsc ? '' : '-') . $field;
      }
      $query['sort'] = implode(",", $sorts);
    }
    $query['page'] = [];
    if ($this->pageSize !== self::DEFAULT_PAGE_SIZE) {
      $query['page']['size'] = $this->pageSize;
    }
    $cursor = $this->pageAfter + $this->pageSize;
    $query['page']['after'] = $cursor;

    return url($path, ['absolute' => TRUE, 'query' => $query]);

  }
}
