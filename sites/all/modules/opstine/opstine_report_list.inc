<?php

function opstine_report_list(){
  $mp=drupal_get_path('module', 'opstine');
  drupal_add_css($mp.'/js/jquery.fancybox.min.css');
  drupal_add_js($mp.'/js/jquery.fancybox.min.js',array('scope' => 'header'));

  drupal_add_js(
    array('izvestaj_list' =>
      array(
        'siteurl' => $GLOBALS['base_url'],
      )
    ), 'setting');

  drupal_add_js($mp.'/js/izvestaj_list.js',array('scope' => 'header'));

  return theme('opstine_report_list', array());
}
function theme_opstine_report_list(){
  global $user;
  $is_admin=false;
  if (in_array('administrator', $user->roles)) {
    $is_admin=true;
  }
  $mp=drupal_get_path('module', 'opstine');
  $account = user_load($user->uid);
  $opstina1 = field_get_items('user', $account, 'field_opstina');
  $opstina=$opstina1[0]['tid'];
  $opstina_term=taxonomy_term_load($opstina);

  $out='';
  $query = db_select('opstine_reports', 't');
  $query->fields('t');
  if(!$is_admin) {
    $query->condition('t.municipality_id', $opstina);
  }
  $query->orderBy('t.report_year', 'DESC');
  $results = $query->execute()->fetchAll();

  if(!$is_admin) {
    $header = array(
        array(
        'data' => t('Godina'),
        'field' => 'f1',
        'sort' => 'desc',
        'datatable_options' => array(
          'bSortable' => FALSE,
          'bSearchable' => TRUE,
        ),
      ),
      array(
        'data' => t('Napomena'),
        'field' => 'f2',
        'datatable_options' => array (
          'bSortable' => FALSE,
          'bSearchable' => TRUE,
        ),
      ),
      array(
        'data' => t('Fajl prihodi'),
        'field' => 'f3',
        'datatable_options' => array(
          'bSortable' => FALSE,
          'bSearchable' => FALSE,
        ),
      ),
      array(
        'data' => t('Fajl rashodi'),
        'field' => 'f4',
        'datatable_options' => array(
          'bSortable' => FALSE,
          'bSearchable' => FALSE,
        ),
      ),
      array(
        'data' => t(''),
        'field' => 'f5',
        'datatable_options' => array (
          'bSortable' => FALSE,
          'bSearchable' => FALSE,
        ),
      ),
    );
  } else {
    $header = array(
      [
        'data' => t('Opština'),
        'field' => 'f0',
        'sort' => 'asc',
        'datatable_options' => [
          'bSortable' => FALSE,
          'bSearchable' => TRUE,
        ],
      ],
      [
        'data' => t('Godina'),
        'field' => 'f1',
        'sort' => 'desc',
        'datatable_options' => [
          'bSortable' => FALSE,
          'bSearchable' => TRUE,
        ],
      ],
      [
        'data' => t('Napomena'),
        'field' => 'f2',
        'datatable_options' => [
          'bSortable' => FALSE,
          'bSearchable' => TRUE,
        ],
      ],
      [
        'data' => t('Fajl prihodi'),
        'field' => 'f3',
        'datatable_options' => [
          'bSortable' => FALSE,
          'bSearchable' => FALSE,
        ],
      ],
      [
        'data' => t('Fajl rashodi'),
        'field' => 'f4',
        'datatable_options' => [
          'bSortable' => FALSE,
          'bSearchable' => FALSE,
        ],
      ],
      [
        'data' => t(''),
        'field' => 'f5',
        'datatable_options' => [
          'bSortable' => FALSE,
          'bSearchable' => FALSE,
        ],
      ],
    );
  }

  $results_new=array();
  foreach ($results as $res){
    $uo=user_load($res->user_created);
    $file_income_uri = file_load($res->file_income_fid);
    $file_income_url = file_create_url($file_income_uri->uri);

    $file_outome_uri = file_load($res->file_outcome_fid);
    $file_outcome_url = file_create_url($file_outome_uri->uri);

    $komende='<div class="komande_wrapp"><span>'.l('<i class="fas fa-edit"></i>', 'opstine_report_form/'.$res->id, array('html'=>true, 'attributes'=>array('title'=>'Uredi izveštaj','class'=>'izv_icon izv_edit'))).'</span><span>'.l('<i class="fas fa-trash-alt"></i>', '#', array('html'=>true,'attributes'=>array('ref'=> $res->id,'title'=>'Ukloni izveštaj','class'=>'izv_icon izv_delete'))).'</span></div>';
    if(!$is_admin) {
      $results_new[] = array( 'data' => array (
        $res->report_year,
        $res->description,
        '<span><a title="pogledaj" href="http://docs.google.com/gview?url=' . $file_income_url . '&embedded=true" class="izv_icon izv_pregled"><i class="fas fa-eye"></i></a></span>' .
        '<span><a title="preuzmi" href="' . $file_income_url . '" download target="_blank" class="izv_icon izv_download"><i class="fas fa-download"></i></a></span>' .
        '<span>' . $file_income_uri->filename . '</span>',

        '<span><a title="pogledaj" href="http://docs.google.com/gview?url=' . $file_outcome_url . '&embedded=true" class="izv_icon izv_pregled"><i class="fas fa-eye"></i></a></span>' .
        '<span><a title="preuzmi" href="' . $file_outcome_url . '" download target="_blank" class="izv_icon izv_download"><i class="fas fa-download"></i></a></span>' .
        '<span>' . $file_outome_uri->filename . '</span>',
        $komende
        ));
    } else {
      $opstinat= taxonomy_term_load($res->municipality_id);
      $results_new[] = array( 'data' => array (
        $opstinat->name,
        $res->report_year,
        $res->description,
        '<span><a title="pogledaj" href="http://docs.google.com/gview?url=' . $file_income_url . '&embedded=true" class="izv_icon izv_pregled"><i class="fas fa-eye"></i></a></span>' .
        '<span><a title="preuzmi" href="' . $file_income_url . '" download target="_blank" class="izv_icon izv_download"><i class="fas fa-download"></i></a></span>' .
        '<span>' . $file_income_uri->filename . '</span>',

        '<span><a title="pogledaj" href="http://docs.google.com/gview?url=' . $file_outcome_url . '&embedded=true" class="izv_icon izv_pregled"><i class="fas fa-eye"></i></a></span>' .
        '<span><a title="preuzmi" href="' . $file_outcome_url . '" download target="_blank" class="izv_icon izv_download"><i class="fas fa-download"></i></a></span>' .
        '<span>' . $file_outome_uri->filename . '</span>',
        $komende
      ));
    }
  }

  $attributes['datatable_options'] = array(
    'bFilter' => TRUE,
    'bInfo' => TRUE,
    'iDisplayLength' => 25,
    'oLanguage' => array(
      'sZeroRecords' => 'Nema podataka za prikaz',
      'sEmptyTable' => 'Vaša tabela je prazna'
    )
  );

  $variables = array(
    'attributes' => $attributes,
    'header' => $header,
    'rows' => $results_new,
  );

  $out .='<h3>'.(!$is_admin ? $opstina_term->name :'Dodavanje').' <span>'.l('<i class="fas fa-plus-circle"></i>','opstine_report_form/0', array('html'=>true, 'attributes'=>array('title'=>'Dodaj novi izveštaj','class'=>'add_report'))).'</span></h3>';

  $out .= '<div class="video_uput">Kratko video uputsvo za postavljanje fajlova pogledajte ';
  $out .= '<a data-fancybox data-width="854" data-height="480" href="'.$mp.'/obrasci/video.mp4"><strong>ovde</strong></a>.<br/>';
  $out .= 'Detaljnija objašnjenja o tabelama <a target="_blank" href="'.$mp.'/obrasci/Uputstvo_za_izradu_otvorenog_budzeta.pdf"><strong>prihoda i rashoda</strong></a>.';
  $out .= '</div>';

  $out .= '<div class="obrasci">Obrasce za dostavu podataka za ';
  $out .= '<a href="'.$mp.'/obrasci/PlanPrihoda.xlsx"><strong>prihode</strong></a> i <a href="'.$mp.'/obrasci/PlanRashoda.xlsx"><strong>rashode</strong></a>';
  $out .= ' preuzmite sa ovih linkova.</div>';

  $out .= theme('datatable', $variables);

  return $out;
}
