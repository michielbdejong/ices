<?php

require_once __DIR__ . "/../ces_komunitin.api.social.inc";

use Neomerx\JsonApi\Schema\BaseSchema;
use Neomerx\JsonApi\Contracts\Schema\ContextInterface;

class File {
  public $id;
  public $url;
  public $filename;

  function __construct($file) {
    $this->id = ces_komunitin_api_social_get_uuid(ResourceTypes::FILE, $file->fid);
    $this->url = $file->url;
    $this->filename = $file->filename;
  }
}

class FileSchema extends BaseSchema {
  public function getType(): string {
    return 'files';
  }
  public function getId($file): ?string {
    assert($file instanceof File);
    return (string) $file->id;
  }
  public function getAttributes($file, ContextInterface $context): iterable {
    assert($file instanceof File);
    $attributes = [
      'url'  => $file->url,
      'filename' => $file->filename,
    ];
    return $attributes;
  }
  public function getRelationships($event, ContextInterface $context): iterable {
    return [];
  }
}
