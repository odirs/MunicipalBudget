<?php
function opstine_mongodb_form($form, &$form_state){
  $form['MongoDB_host'] = array(
    '#type' => 'textfield',
    '#title' => t('Host'),
    '#default_value' => variable_get('MongoDB_host', '127.0.0.1'),
    '#required' => TRUE,
  );
  $form['MongoDB_port'] = array(
    '#type' => 'textfield',
    '#title' => t('Port'),
    '#default_value' => variable_get('MongoDB_port', '27017'),
    '#required' => TRUE,
  );
  $form['MongoDB_user'] = array(
    '#type' => 'textfield',
    '#title' => t('User'),
    '#default_value' => variable_get('MongoDB_user', ''),
  );
  $form['MongoDB_password'] = array(
    '#type' => 'textfield',
    '#title' => t('Password'),
    '#default_value' => variable_get('MongoDB_password', ''),
  );
  $form['MongoDB_database'] = array(
    '#type' => 'textfield',
    '#title' => t('Database'),
    '#default_value' => variable_get('MongoDB_database', 'undp'),
    '#required' => TRUE,
  );

  return system_settings_form($form);
}

