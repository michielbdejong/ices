<?php

use Neomerx\JsonApi\Http\Query\BaseQueryParser;

class JsonApiRequest {
  /**
   * Uppercase http request method.
   */
  public $method;
  public $path;

  public $includes;
  public $fields;
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

    if ($this->method == 'POST' || $this->method == 'PATCH') {
      $this->body = file_get_contents('php://input');
    }
  }
}
