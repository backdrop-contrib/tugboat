<?php
/**
 * @file
 * Administrative pages and callbacks for the Tugboat module.
 */

/**
 * The Tugboat administration form.
 */
function tugboat_admin_settings($form, &$form_state) {
  $config = config('tugboat.settings');
  $form['#config'] = 'tugboat.settings';

  $form['tugboat.qa'] = array(
    '#type' => 'fieldset',
    '#title' => t('Tugboat.qa details'),
  );
  $token = settings_get('tugboat_access_token');
  if (!empty($token)) {
    $form['tugboat.qa']['access_token'] = array(
      '#type' => 'item',
      '#title' => t('Access token'),
      '#description' => t("An <a href=\"@url\">access token</a> generated from Tugboat.qa. This has already been set in 'settings.php'.", array('@url' => 'https://dashboard.tugboat.qa/access-tokens')),
      '#markup' => substr($token, 0, 4) . str_repeat('*', strlen($token) - 8) . substr($token, -4),
    );
  }
  else {
    $form['tugboat.qa']['access_token'] = array(
      '#type' => 'textfield',
      '#title' => t('Access token'),
      '#description' => t("An <a href=\"@url\">access token</a> generated from Tugboat.qa. It can alternatively be set in 'settings.php' via <code>\$settings['tugboat_access_token']</code>.", array('@url' => 'https://dashboard.tugboat.qa/access-tokens')),
      '#default_value' => $config->get('access_token'),
      '#size' => 40,
    );
  }
  $form['tugboat.qa']['repository_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Repository ID'),
    '#description' => t("The ID of your Tugboat repository. This 24-character, alphanumeric hash can be found in the URL when viewing the repository page in the Tugboat.qa dashboard (e.g. <code>5e177759fa8a5e0f9ae64d14</code>)."),
    '#default_value' => $config->get('repository_id'),
    '#size' => 30,
  );
  $form['tugboat.qa']['base_preview_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Base preview ID'),
    '#description' => t("The ID of the Tugboat base preview to use for cloning new previews. This 24-character, alphanumeric hash can be found in the URL when viewing the preview page in the Tugboat.qa dashboard (e.g. <code>5e177775fa8a5e61a7e64d53</code>)."),
    '#default_value' => $config->get('base_preview_id'),
    '#size' => 30,
  );
  $form['this_site'] = array(
    '#type' => 'fieldset',
    '#title' => t('Settings for this site'),
  );
  $form['this_site']['command_line_tool'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to the command line tool'),
    '#description' => t("The path to the Tugboat.qa <a href=\"@url\">command line tool</a> on the server. Can be an absolute path, or relative to the Backdrop installation.", array('@url' => 'https://dashboard.tugboat.qa/downloads')),
    '#default_value' => $config->get('command_line_tool'),
  );
  $form['this_site']['page_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Page URL'),
    '#description' => t("The URL of the 'Create' and 'Ready' pages for this site."),
    '#default_value' => $config->get('page_url'),
    '#field_prefix' => $_SERVER['SERVER_NAME'] . '/',
  );
  $form['this_site']['create_page_title'] = array(
    '#type' => 'textfield',
    '#title' => t("'Create' page title"),
    '#description' => t("The title of the 'Create' page for this site (where people go to create Tugboat preview sites)."),
    '#default_value' => $config->get('create_page_title'),
  );
  $form['this_site']['ready_page_title'] = array(
    '#type' => 'textfield',
    '#title' => t("'Ready' page title"),
    '#description' => t("The title of the 'Ready' page for this site (where people are taken when their Tugboat preview site is ready)."),
    '#default_value' => $config->get('ready_page_title'),
  );
  $form['this_site']['preview_site_age'] = array(
    '#type' => 'select',
    '#title' => t('Preview site age'),
    '#description' => t("How long a preview site will be available for. Preview sites older than this will be automatically deleted during cron runs."),
    '#default_value' => $config->get('preview_site_age'),
    '#options' => backdrop_map_assoc(array(
      3600,    // 1 hour
      10800,   // 3 hours
      21600,   // 6 hours
      43200,   // 12 hours
      86400,   // 1 day
      172800,  // 2 days
      259200,  // 3 days
      432000,  // 5 days
      604800,  // 1 week
      1209600, // 2 weeks
      2592000, // 1 month
    ), 'format_interval'),
  );

  $form['#validate'] = array('tugboat_admin_settings_validate');
  return system_settings_form($form);
}

/**
 * Validate handler for tugboat_admin_settings().
 */
function tugboat_admin_settings_validate($form, &$form_state) {
  $token = !empty($form_state['values']['access_token']) ? $form_state['values']['access_token'] : NULL;
  $path = $form_state['values']['command_line_tool'];

  // Validate the path to the command line tool.
  $error = '';
  $valid = _tugboat_validate_path($path, $error);
  if (!$valid) {
    form_set_error('command_line_tool', $error);
    return;
  }

  // Validate the Repository ID.
  $return = array();
  $error = '';
  $success = _tugboat_execute('find ' . $form_state['values']['repository_id'], $return, $error, $token, $path);
  if (!$success) {
    if ($error) {
      form_set_error('repository_id', t('The provided repository ID was not found. Error from Tugboat: @error', array('@error' => check_plain($error))));
    }
    else {
      form_set_error('repository_id', t('Tugboat returned a response that was not understood.'));
    }
    return;
  }

  // Validate the Base Preview ID.
  $return = array();
  $error = '';
  $success = _tugboat_execute('find ' . $form_state['values']['base_preview_id'], $return, $error, $token, $path);
  if (!$success) {
    if ($error) {
      form_set_error('base_preview_id', t('The provided base preview ID was not found. Error from Tugboat: @error', array('@error' => '<code>' . check_plain($error) . '</code>')));
    }
    else {
      form_set_error('base_preview_id', t('Tugboat returned a response that was not understood.'));
    }
    return;
  }
}
