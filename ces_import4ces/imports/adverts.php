<?php
/**
 * @file
 * Functions from parse adverts and category
 */

/**
 * @defgroup ces_import4ces_adverts Parse offers and wants from CES
 * @ingroup ces_import4ces
 * @{
 * Functions from parse offers and wants
 */

/**
 * Parse adverts.
 */
function ces_import4ces_parse_adverts($import_id, $data, $row, &$context, $width_ajax = TRUE) {

  $offer = FALSE;

  if (isset($context['results']['error'])) {
    return;
  }

  $context['results']['import_id'] = $import_id;
  $import = ces_import4ces_import_load($import_id);
  $import->row = $row;

  $tx = db_transaction();
  try {
    ob_start();

    // Find uid from user.
    $query = db_query('SELECT uid FROM {users} where name=:name', array(':name' => $data['uid']));
    $offer_user_id = $query->fetchColumn(0);

    // If the user is not on the network is skipped and saved report.
    if (empty($offer_user_id) || !$offer_user_id) {
      ces_save_discarded_record($import_id, $data, 'The user is not network');
    }
    else {


      // Having no category offers, create one for which indicates the lack of it.
      $category = !empty($data['category']) ? $data['category'] : 'unclassified';

      $category_id = ces_import4ces_get_category($category, $import);

      $ad_type = array(
        'o' => 'offer',
        'w' => 'want'
      );

      $offer = array(
        'type' => $ad_type[$data['ad_type']],
        'user' => $data['uid'],
        'title' => $data['title'],
        'body' => $data['description'],
        'category' => $category_id,
        'keywords' => $data['keywords'],
        'state' => (($data['hide'] == 0) ? 1 : 0),
        'created' => strtotime($data['date_added']),
        'modified' => time(),
        'expire' => strtotime($data['date_expires']),
        'rate' => $data['talent_rate'],
        'image' => $data['image'],
      );

      $extra_info = $data;

      if (empty($offer_user_id) || !$offer_user_id) {
        $m = t('The user @user was not found in advert import row @row. It may be a
        user from another exchange not yet imported.', array('@user' => $data['uid'], '@row' => $row));
        $context['results']['error'] = $m;
        throw new Exception($m);
      }

      $offer['user'] = $offer_user_id;

      $o = (object) $offer;
      $o->ces_offer_rate = array(LANGUAGE_NONE => array(array('value' => $offer['rate'])));
      unset($o->rate);
      $offer = ces_offerwant_save($o);

      if (!empty($offer->image)) {

        $file = 'https://www.community-exchange.org/pics/' . $data['image'];
        $parts = explode(".", $file);
        $extension = end($parts);
        $directory = file_default_scheme() . '://' . variable_get('ces_offerswants_picture_path', 'ces_offerswants_pictures');
        file_prepare_directory($directory, FILE_CREATE_DIRECTORY);
        $name_image = 'picture-' . $offer->id . '-' . REQUEST_TIME . '.' . $extension;
        $destination = file_stream_wrapper_uri_normalize($directory . '/' . $name_image);
        if (ces_import4ces_download_image($file, $destination)) {
          $file = new stdClass();
          $file->uid = $offer_user_id;
          $file->filename = $name_image;
          $file->uri = $destination;
          $file->status = 1;
          $file->filemime = image_type_to_mime_type(exif_imagetype($destination));
          file_save($file);
          file_usage_add($file, 'ces_offerswants', 'ces_offerwant', $offer->id);
          $offer->image = $file->fid;
          // Re-save the entity with the new image file id.
          $offer = ces_offerwant_save($offer);
        }
        else {
          throw new Exception(t('Image @file could not be downloaded.', array('@file' => $file)));
        }
      }

    }

    if ( $offer ) {
      db_insert('ces_import4ces_objects')->fields(
        array(
          'import_id' => $import_id,
          'object' => 'offers',
          'object_id' => $offer->id,
          'row' => $row,
          'data' => serialize($extra_info),
        ))->execute();
    }
    ces_import4ces_update_row($import_id, $row);
    ob_end_clean();
  }
  catch (Exception $e) {
    ob_end_clean();
    $tx->rollback();
    $context['results']['error'] = check_plain($e->getMessage());
    $_SESSION['ces_import4ces_row_error']['row']  = $row;
    $_SESSION['ces_import4ces_row_error']['m']    = $e->getMessage();
    $_SESSION['ces_import4ces_row_error']['data'] = $data;
    if ($width_ajax) {
      $result = array('status' => FALSE, 'data' => check_plain($e->getMessage()));
      die(json_encode($result));
    }
    else {
      ces_import4ces_batch_fail_row($import_id, array_keys($data), array_values($data), $row, $context);
    }
  }
}
/** @} */
