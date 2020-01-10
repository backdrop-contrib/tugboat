<?php
/**
 * Menu callbacks for Tugboat module pages.
 */

/**
 * Page for creating a preview site.
 */
function tugboat_preview_create_page() {
  // Disable caching on this page, needed for AJAX form submitting working with
  // the page cache.
  backdrop_page_is_cacheable(FALSE);

  return theme('tugboat_preview_create_page', array(
    'form' => backdrop_get_form('tugboat_preview_create_form'),
  ));
}

/**
 * Page for linking to the preview site.
 */
function tugboat_preview_ready_page() {
  return theme('tugboat_preview_ready_page', array(
    'url' => isset($_GET['url']) ? $_GET['url'] : 'Unknown',
    'duration' => format_interval(config_get('tugboat.settings', 'preview_site_age')),
  ));
}

/**
 * Provide a form that creates a new preview site.
 */
function tugboat_preview_create_form($form, &$form_state) {
  // Tugboat repository ID, a 24 character hash.
  $form['repo'] = array(
    '#type' => 'value',
    '#value' => config_get('tugboat.settings', 'repository_id'),
  );
  $form['ref_id'] = array(
    '#type' => 'value',
    '#value' => config_get('tugboat.settings', 'base_preview_id'),
  );
  // Date added to the created preview site for reference.
  $form['date'] = array(
    '#type' => 'value',
    '#value' => format_date(REQUEST_TIME, 'custom', 'Y-m-d'),
  );
  $form['create_preview_site'] = array(
    '#type' => 'submit',
    '#value' => t('Create preview site'),
    // '#validate' => array('tugboat_preview_create_form_validate'),
    '#submit' => array('tugboat_preview_create_form_submit'),
    '#attributes' => array('class' => array('button', 'button-large', 'preview-site')),
    '#ajax' => array(
      'progress' => array(
        'type' => 'throbber',
        'message' => t('Creating preview site...'),
      ),
      'callback' => 'tugboat_preview_create_form_ajax',
    ),
  );

  return $form;
}

/**
 * Check if the base preview exists.
 */
// function tugboat_preview_create_form_validate($form, &$form_state) {
//   $repo = $form_state['values']['repo'];
//
//   // Get a list of all current previews to see if a designated base preview
//   // exists for the given branch.
//   $base_found = FALSE;
//   $return = array();
//   $error = '';
//   _tugboat_execute("ls previews repo=$repo", $return, $error);
//   if ($error) {
//     form_set_error(NULL, t('A preview site cannot be created at this time.'));
//     watchdog('tugboat', 'A preview site failed to be created, error from Tugboat: @error', array('@error' => $error));
//     return;
//   }
//
//   foreach ($return as $preview) {
//     // Only compare against base previews.
//     if (!$preview['anchor']) {
//       continue;
//     }
//
//     if ($preview['name'] === 'master') {
//       // Store the Tugboat base preview ID for use in cloning.
//       $form_state['values']['ref_id'] = $preview['id'];
//       $base_found = TRUE;
//     }
//   }
//
//   if (!$base_found) {
//     form_set_error('branch', t('The tag or branch "@ref" could not be found for creating a demo sandbox.', array('@ref' => $ref)));
//     watchdog('tugboat', 'A sandbox with the ref "@ref" failed to be created because no base preview with that name could be found', array('@ref' => $ref));
//   }
// }

/**
 * Submit handler for tugboat_preview_create_form().
 */
function tugboat_preview_create_form_submit($form, &$form_state) {
  $repo = $form_state['values']['repo'];
  $ref_id = $form_state['values']['ref_id'];
  $date = $form_state['values']['date'];

  $return = array();
  $error = '';

  $status = _tugboat_execute("clone $ref_id label=one-click-demo--$date", $return, $error);

  if (!$status) {
    backdrop_set_message(t('A preview site could not be created at this time! Please try again later.'));
    if ($error) {
      watchdog('tugboat', 'Failed to create preview site. Error from Tugboat: @error', array('@error' => $error));
    }
    else {
      watchdog('tugboat', 'Failed to create preview site. No error data returned from Tugboat!');
    }
  }
  else {
    backdrop_set_message(t('New preview site creation successful.'));
    $form_state['redirect'] = array(config_get('tugboat.settings', 'page_url') . '/ready', array('query' => array(
      'url' => $return['url'],
    )));
  }
}

function tugboat_preview_create_form_ajax($form, $form_state) {
  $commands = array();

  if (isset($form_state['redirect'])) {
    $url = url($form_state['redirect'][0], $form_state['redirect'][1]);
    $commands[] = ajax_command_redirect($url);
  }
  else {
    $commands[] = ajax_command_remove('.demo-create-page .messages');
    $commands[] = ajax_command_prepend('.demo-create-page', theme('status_messages'));
  }

  return array('#type' => 'ajax', '#commands' => $commands);
}
