<?php

/**
 * @file
 * This file provides drush integration for hosting_vnodectrl.
 * It communiciates with the vnodectrl drush backend and provides
 * it with information it needs.
 */

/**
 * Implementation of hook_drush_command().
 */
function hosting_vnodectrl_drush_command() {
  $items = array();
  $items['hosting-vnodectrl-register-images'] = array(
    'description' => dt('Imports available images throgh vnodectrl.'),
  );
  return $items;
}

/**
 * Callback for hosting-vnodectrl-register-images.
 * Register all available images for drivers that exist.
 */
function drush_hosting_vnodectrl_register_images() {
  drush_log("Fetching images", 'status');
  drush_shell_exec("vnodectrl list-images --format=json");
  $images_output = implode('\n', drush_shell_exec_output());
  $images = (array)json_decode($images_output);
  if (is_array($images)) {
    // Empty the database
    db_query("DELETE FROM {hosting_vnodectrl_images}");
    foreach ($images as $driver => $images) {
      foreach ($images as $image) {
        $image->provider = $driver;
        drupal_write_record('hosting_vnodectrl_images', $image);
      }
    }
    drush_log("Images updated.", 'ok');
  }
  else {
    drush_log('Something went wrong: ' . json_last_error(), 'error');
  }

}

function drush_hosting_vnodectrl_pre_hosting_task() {
  $task =& drush_get_context('HOSTING_TASK');
  if ($task->ref->type == 'server' && ($task->task_type == 'create-server' || $task->task_type == 'delete-server')) {
    drush_log(print_r($task->ref, TRUE), 'ok');
    $task->options['provider'] = $task->ref->provider;
    $task->options['image'] = $task->ref->image;
    $task->options['name'] = $task->ref->title;
    $task->options['size'] = $task->ref->size;
  }
  else {
    drush_log("something went wrong", 'error');
  }
}

function drush_hosting_vnodectrl_post_hosting_task() {
  $task = & drush_get_context('HOSTING_TASK');
  if ($task->ref->type == 'server' && $task->task_type == 'create-server') {
    $output =& drush_get_context('HOSTING_DRUSH_OUTPUT');
    drush_log(print_r($output['object'], TRUE), 'ok');
    if (isset($output['object']['node']['id'])) {
      // Save the machine ID so we can operate on it.
      $task->ref->instance_id = $output['object']['node']['id'];
      node_save($task->ref);
    }
  }
}
