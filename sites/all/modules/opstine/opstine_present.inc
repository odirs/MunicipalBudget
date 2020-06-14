<?php

function opstine_report_present(){
  $mp=drupal_get_path('module', 'opstine');

  drupal_add_css($mp.'/css/opstine_report_present.css');
  drupal_add_css($mp.'/css/leaflet.css');
  drupal_add_js(
    array('opstine' =>
      array(
        'datapath' => $mp.'/jsons/opstine.geojson',
        'siteurl' => $GLOBALS['base_url'],
      )
    ), 'setting');

  //drupal_add_js($mp.'/js/d3v5.min.js',array('scope' => 'header'));
  drupal_add_js($mp.'/js/leaflet.js',array('scope' => 'header'));
  drupal_add_js($mp.'/js/opstine_report_present1.js',array('scope' => 'footer'));
  return theme('opstine_report_present', array());
}

function theme_opstine_report_present(){
  $block = block_load('block', '1');
  $output =_block_get_renderable_array(_block_render_blocks(array($block)));
  $output = drupal_render($output);
  $out ='<div class="container">';
  $out .='<div class="row">';
  $out .='<div id="srbija_intro_text" class="col-sm" >'.$output.'</div>';
  $out .='<div id="srbija_opstine_map" class="col-sm" style="height: 700px"></div>';
  $out .='</div>';
  $out .='</div>';
  return $out;
}

