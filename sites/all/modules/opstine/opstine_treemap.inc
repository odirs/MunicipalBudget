<?php

function opstine_report_tree($oid){
    $sql = "SELECT municipality_id, report_year FROM {opstine_reports} WHERE municipality_id=".$oid." ORDER BY report_year DESC";
    $r = db_query($sql);
    $results = $r->fetchAll();
    $last_year=0;
    foreach ($results as $result){
        $last_year=$result->report_year;
    }
    $mp=drupal_get_path('module', 'opstine');
    $datapath = file_create_url(file_build_uri('opstine_data'));
    $datapath_json = file_create_url(file_build_uri('json_for_mongo'));
    $datapath1 = file_build_uri('json_for_mongo/'.$oid.'_'.$last_year.'_outcome_treemap.json');
    $datapath2 = $datapath_json.'/'.$oid.'_'.$last_year.'_outcome_treemap.json';
    drupal_add_css($mp.'/css/opstine.css');
    drupal_add_css($mp.'/css/treemap.css');

    drupal_add_js(
        array('opstine' =>
            array(
                'datapath' => $datapath2,
                'siteurl' => $GLOBALS['base_url'],
            )
        ), 'setting');
    drupal_add_js($mp.'/js/underscore-min.js',array('scope' => 'header'));
    drupal_add_js($mp.'/js/d3v3.min.js',array('scope' => 'header'));
    drupal_add_js('root='.file_get_contents($datapath1), array('type'=>'inline','scope' => 'header'));
    //drupal_add_js($mp.'/js/opstine_budzet.js',array('scope' => 'header'));
    drupal_add_js($mp.'/js/treemap.js',array('scope' => 'header'));

    return theme('opstine_report_tree',array('oid'=>$oid, 'last_year'=>$last_year));
}

function theme_opstine_report_tree($vars){
    global $user;
    $op=taxonomy_term_load($vars['oid']);
    drupal_set_title('Opština '.$op->name .' - plan rashoda za '.$vars['last_year'].'. godinu' );
    $out='';
    $out.='<div id="tree_wrapper">';
    $out.='</div>';
    $out.='<div class="col-md-23 legend" style="margin-top:-10px;margin-bottom:0px;">

	<div id="legend"></div>
</div>';





    return $out;
}
