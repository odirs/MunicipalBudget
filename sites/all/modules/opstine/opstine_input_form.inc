<?php
include_once "opstine_parser.inc";

function opstine_input_form($form, &$form_state, $report_id){

  global $user;
  $is_admin=false;
  if (in_array('administrator', $user->roles)) {
    $is_admin=true;
  }

  //  opstine_visualization_data_from_file(2012, 2013);
  $account = user_load($user->uid);
  $opstina1 = field_get_items('user', $account, 'field_opstina');
  $opstina=$opstina1[0]['tid'];
  $opstina_term=taxonomy_term_load($opstina);
  $years = array_combine(range(intval(date("Y")), intval(date("Y"))-30), range(intval(date("Y")), intval(date("Y"))-30));
  $result = null;
  if ($report_id != 0) {
    $query = db_select('opstine_reports', 't')
      ->fields('t')
      ->condition('t.id', $report_id)
      ->range(0, 1)
      ->execute();
    $result = $query->fetchObject();
    if (!$result) {
      drupal_set_message('Izveštaj nije pronađen!', 'error');
      return $form;
    }
  }

  $form['id'] = [
    '#type' => 'value',
    '#value' => $report_id,
  ];

  if(!$is_admin) {
    $form['municipality_id'] = array(
      '#type' => 'value',
      '#value' => $opstina,
    );
  } else {
    $sov = taxonomy_vocabulary_machine_name_load('opstine');
    $so = taxonomy_get_tree($sov->vid);
    $sve_opstine=array();
    foreach ($so as $ops) {
      $sve_opstine[$ops->tid]=$ops->name;
    }

    $form['municipality_id'] = array(
      '#type' => 'select',
      '#title' => t('Opština'),
      '#options' => $sve_opstine,
      '#default_value'=> ($report_id!=0 ? $result->municipality_id : $opstina),
    );
  }

  $form['report_year'] = array(
    '#type' => 'select',
    '#title' => t('Godina'),
    '#options' => $years,
    '#default_value' => isset($result->report_year) ? $result->report_year : (intval(date("Y"))),
  );

  $form['file_income_fid'] = array(
    '#title' => t('Dokument o prihodima'),
    '#type' => 'managed_file',
    '#required' => true,
    '#default_value' => isset($result->file_income_fid) ? $result->file_income_fid : '',
    '#upload_location' => 'public://dokumenti/',
    '#upload_validators' => array(
      'file_validate_extensions' => array('xls xlsx txt csv'),
    ),
  );

  $form['file_outcome_fid'] = array(
    '#title' => t('Dokument o rashodima'),
    '#type' => 'managed_file',
    '#required' => true,
    '#default_value' => isset($result->file_outcome_fid) ? $result->file_outcome_fid : '',
    '#upload_location' => 'public://dokumenti/',
    '#upload_validators' => array(
      'file_validate_extensions' => array('xls xlsx txt csv'),
    ),
  );

  $form['description'] = array(
    '#title' => t('Napomena'),
    '#type' => 'textarea',
    '#default_value' => isset($result->description) ? $result->description : '',
  );
  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => $report_id ? t('Izmeni') : t('Kreiraj izveštaj'),
      '#attributes' => [
        'class' => ['chart_more_link'],
      ],
    ),
    'cancel' => array(
      '#markup' => l(t('Odustani'), 'opstine_report_list'),
    )
  );
  return $form;
}

function opstine_input_form_submit($form, &$form_state){
  global $user;
  $who = $user->uid;
  $id=$form_state['values']['id'];
  $municipality_id = $form_state['values']['municipality_id'];
  $report_year=$form_state['values']['report_year'];

  if (isset($form_state['values']['file_income_fid']) && $form_state['values']['file_income_fid'] != 0) {
    if ($form_state['values']['file_income_fid']) {
      $file1 = file_load($form_state['values']['file_income_fid']);

      if ($file1) {
        $file1->status = FILE_STATUS_PERMANENT;
        file_save($file1);
        file_usage_add($file1, 'opstine_module', 'opstine_module', $file1->uid);
        $in_val=opstine_parser_main($report_year, $municipality_id, "income", $file1->uri);
      }
    }
  }

  if (isset($form_state['values']['file_outcome_fid']) && $form_state['values']['file_outcome_fid'] != 0) {
    if ($form_state['values']['file_outcome_fid']) {
      $file2 = file_load($form_state['values']['file_outcome_fid']);
      if ($file2) {
        $file2->status = FILE_STATUS_PERMANENT;
        file_save($file2);
        file_usage_add($file2, 'opstine_module', 'opstine_module', $file2->uid);
        $out_val=opstine_parser_main($report_year, $municipality_id, "outcome", $file2->uri);
      }
    }
  }

  if($in_val+$out_val==2) {
    if ($id) {
      $insert_data = db_update('opstine_reports')->fields([
        'report_year' => $report_year,
        'municipality_id' => $form_state['values']['municipality_id'],
        'description' => $form_state['values']['description'],
        'file_income_fid' => $form_state['values']['file_income_fid'],
        'file_outcome_fid' => $form_state['values']['file_outcome_fid'],
        'user_updated' => $who,
      ])->condition('id', $id, '=')->execute();
    }
    else {
      $insert_data = db_insert('opstine_reports')->fields([
        'report_year',
        'municipality_id',
        'description',
        'file_income_fid',
        'file_outcome_fid',
        'user_created',
      ]);
      $insert_data->values([
        'report_year' => $report_year,
        'municipality_id' => trim($form_state['values']['municipality_id']),
        'description' => $form_state['values']['description'],
        'file_income_fid' => $form_state['values']['file_income_fid'],
        'file_outcome_fid' => $form_state['values']['file_outcome_fid'],
        'user_created' => $who,
      ]);
      $last_insert_id = $insert_data->execute();
    }

    drupal_goto('opstine_report_list');
  } else {
    drupal_set_message('Podaci nisu sačuvani zbog neispravnosti tabela.', 'error');
  }
}
