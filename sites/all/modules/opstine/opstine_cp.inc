<?php

function opstine_cp($oid, $pord){
  $sql = "SELECT municipality_id, report_year FROM {opstine_reports} WHERE municipality_id=".$oid." ORDER BY report_year DESC";
  $r = db_query($sql);
  $results = $r->fetchAll();
  $last_year=0;
  foreach ($results as $result){
    $last_year=$result->report_year;
  }

  $mp=drupal_get_path('module', 'opstine');
  $datapath = file_create_url(file_build_uri('opstine_data'));
  $datapath1 = file_create_url(file_build_uri('json_for_mongo/'.$oid.'_'.$last_year.'_'.$pord.'_cp.json'));
  drupal_add_css($mp.'/css/opstine_cp.css');

  drupal_add_js($mp.'/js/d3v4.min.js',array('scope' => 'header'));
  //drupal_add_js("https://d3js.org/d3.v4.min.js", array('type'=>'external', 'scope'=>'footer', 'weight' => 5));
  drupal_add_js('var root_in="'.$datapath1.'";', array('type'=>'inline','scope' => 'header'));
  drupal_add_js($mp.'/js/opstine_cp.js',array('scope' => 'footer','weight' => 6));

	return theme('opstine_cp',array('oid'=>$oid, 'last_year'=>$last_year, 'pord'=>$pord));
}

function theme_opstine_cp($vars){
  global $user;
  $plan='';
  if ($vars['pord']=='income') $plan='prihoda';
  if ($vars['pord']=='outcome') $plan='rashoda';
  $op=taxonomy_term_load($vars['oid']);
  drupal_set_title('Opština '.$op->name .' - plan '.$plan.' za '.$vars['last_year'].'. godinu' );
  $out='';
  $out.='
  <div id="chart_wrapper" class="chart_wrapper_cp">
    <svg width="960" height="960"></svg>
  </div>';
	return $out;
}
