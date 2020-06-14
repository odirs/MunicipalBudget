<?php

function opstine_front($oid, $type){
  $sql = "SELECT municipality_id, report_year FROM {opstine_reports} WHERE municipality_id=".$oid." ORDER BY report_year DESC";
  $r = db_query($sql);
  $results = $r->fetchAll();
  $last_year=0;
  foreach ($results as $result){
    $last_year=$result->report_year;
  }

  $mp=drupal_get_path('module', 'opstine');
  $datapath = file_create_url(file_build_uri('opstine_data'));
  $datapath1 = file_build_uri('json_for_mongo/'.$oid.'_'.$last_year.'_'.$type.'.json');
  $data_file_content=file_get_contents($datapath1);
  drupal_add_css($mp.'/js/bootstrap-drawer.min.css');
  drupal_add_css($mp.'/css/opstine.css');

  drupal_add_js(
    array('opstine' =>
      array(
        'datapath' => $datapath,
        'siteurl' => $GLOBALS['base_url'],
      )
    ), 'setting');

  drupal_add_js($mp.'/js/drawer.min.js',array('scope' => 'header'));
  drupal_add_js($mp.'/js/underscore-min.js',array('scope' => 'header'));
  drupal_add_js($mp.'/js/d3v3.min.js',array('scope' => 'header'));
  drupal_add_js('root='.$data_file_content, array('type'=>'inline','scope' => 'header'));
  drupal_add_js($mp.'/js/opstine_budzet.js',array('scope' => 'header'));

	return theme('opstine_front',array('oid'=>$oid, 'last_year'=>$last_year, 'type'=>$type, 'file_content'=>$data_file_content));
}

function theme_opstine_front($vars){
  $programi=array(
        '1101'=>array("ime"=>"СТАНОВАЊЕ, УРБАНИЗАМ И ПРОСТОРНО ПЛАНИРАЊЕ", "boja"=>"#ff8c26"),
        '1102'=>array("ime"=>"КОМУНАЛНЕ ДЕЛАТНОСТИ", "boja"=>"#ffc185"),
        '1501'=>array("ime"=>"ЛОКАЛНИ ЕКОНОМСКИ РАЗВОЈ", "boja"=>"#41a941"),
        '1502'=>array("ime"=>"РАЗВОЈ ТУРИЗМА", "boja"=>"#c2c338"),
        '0101'=>array("ime"=>"ПОЉОПРИВРЕДА И РУРАЛНИ РАЗВОЈ", "boja"=>"#c9b6d9"),
        '0401'=>array("ime"=>"ЗАШТИТА ЖИВОТНЕ СРЕДИНЕ", "boja"=>"#ed9a53"),
        '0701'=>array("ime"=>"ОРГАНИЗАЦИЈА САОБРАЋАЈА И САОБРАЋАЈНА ИНФРАСТРУКТУРА", "boja"=>"#da3c3d"),
        '2001'=>array("ime"=>"ПРЕДШКОЛСКО ВАСПИТАЊЕ И ОБРАЗОВАЊЕ", "boja"=>"#fda1a0"),
        '2002'=>array("ime"=>"ОСНОВНО ОБРАЗОВАЊЕ И ВАСПИТАЊЕ", "boja"=>"#77af9d"),
        '2003'=>array("ime"=>"СРЕДЊЕ ОБРАЗОВАЊЕ И ВАСПИТАЊЕ", "boja"=>"#e584c8"),
        '0901'=>array("ime"=>"СОЦИЈАЛНА И ДЕЧИЈА ЗАШТИТА", "boja"=>"#f7bdd6"),
        '1801'=>array("ime"=>"ЗДРАВСТВЕНА ЗАШТИТА", "boja"=>"#777777"),
        '1201'=>array("ime"=>"РАЗВОЈ КУЛТУРЕ И ИНФОРМИСАЊА", "boja"=>"#acacac"),
        '1301'=>array("ime"=>"РАЗВОЈ СПОРТА И ОМЛАДИНЕ", "boja"=>"#a1e095"),
        '0602'=>array("ime"=>"ОПШТЕ УСЛУГЕ ЛОКАЛНЕ САМОУПРАВЕ", "boja"=>"#8da2bf"),
        '2101'=>array("ime"=>"ПОЛИТИЧКИ СИСТЕМ ЛОКАЛНЕ САМОУПРАВЕ", "boja"=>"#B4CBEA"),
        '0501'=>array("ime"=>"ЕНЕРГЕТСКА ЕФИКАСНОСТ И ОБНОВЉИВИ ИЗВОРИ ЕНЕРГИЈЕ","boja"=>"#193ffe")
  );

  global $user;
  $op=taxonomy_term_load($vars['oid']);
  $type=$vars['type'];
  $file_content=json_decode($vars['file_content']);

  $programss=array();
  $programssum=0;
  foreach ($file_content->budget_data as $rz){
    foreach ($rz->razdel_programs as $rp){
      if(isset($programss[$rp->program_id])) {
        $programss[$rp->program_id]+=$rp->total;
      } else {
        $programss[$rp->program_id]=$rp->total;
      }
      $programssum+=$rp->total;
    }
  }
  $drawer='';
  if($type=='outcome'){
    $drawer.='<table id="po_programima"><tr><th style="text-align: center">ШИФРА ПРОГРАМА</th><th style="text-align: center">НАЗИВ ПРОГРАМА</th>
            <th style="text-align: center">СРЕДСТВА[рсд]</th><th style="text-align: center">УЧЕШЋЕ У БУЏЕТУ[%]</th></tr>';
    foreach ($programi as $kpr=>$kval){
      $pval=isset($programss[$kpr]) ? $programss[$kpr] : 0;
      $drawer.='<tr style="color:'.$kval['boja'].'"><td style="text-align: center">'.$kpr.'</td>
            <td>'.$kval['ime'].'</td>
            <td style="text-align: right">'.number_format($pval).'</td>
            <td style="text-align: right">'.number_format($pval/$programssum*100,2).'</td></tr>';
    }
    $drawer.='<tr><td></td><td style="text-align: right; font-weight: bold">УКУПНО:</td>
            <td style="text-align: right; font-weight: bold">'.number_format($programssum).'</td><td style="text-align: right; font-weight: bold">100.00</td></tr>';
    $drawer.='</table>';
  }

  drupal_set_title('Општина '.$op->name .' - план '.(($type=='outcome')?'расхода' : 'прихода').' за '.$vars['last_year'].'. годину' );
  $out='';
  $out.='
  
    <table>
      <tbody>';
  $out.='<tr>
        <td width="200px"><div class="dropdown">
          <button class="btn btn-default dropdown-toggle" type="button" id="dropdownMenu1" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
            Изаберите приказ
            <span class="caret"></span>
          </button>
          <ul class="dropdown-menu" aria-labelledby="dropdownMenu1">';
          $out.='<li><a href="javascript:jQuery.draw(\'l1\')">Неразврстано</a></li>';
          $out.='<li><a href="javascript:jQuery.draw(\'program_name\')">'.( ($type!='outcome') ? 'По категоријама': 'По програмима').'</a></li>';
          $out.='<li><a href="javascript:jQuery.draw(\'activity_id\')">'.( ($type!='outcome') ? 'По групама': 'По програмској класификацији').'</a></li>';
          $out.='<li><a href="javascript:jQuery.draw(\'ek_number\')">По класификацијама</a></li>';
          //$out.='<li><a href="javascript:jQuery.draw(\'razlike\')">Разлике у односу на претходну годину</a></li>';
  $out.='</ul>
        </div></td>
        <td style="width:70px; color:#0D4077; padding: 20px 0; text-align: right"><b>ИЗВОРИ:</b></td>
        <td><div class="btn-group" data-toggle="buttons">
          <label class="btn btn-default active" title="Приходи из буџета"><input type="checkbox" id="i01" checked="" autocomplete="off">
            Буџетска средства</label>
          <label class="btn btn-default active" title="Сопствени приходи буџетских корисника"><input type="checkbox" id="i02" checked="" autocomplete="off">
            Сопствена средства</label>
          <label class="btn btn-default active" title="Остало"><input type="checkbox" id="i03" checked="" autocomplete="off">Остало</label>
        </div></td>

      </tr>
        </tbody></table>
      ';
  $out.='
        <div id="infoch" style="display: none;">
            <p class="oznaka" style="top:50px;">+25% и више</p>
            <p class="nivo" style="top:100px;">+20%</p>
            <p class="nivo" style="top:200px;">+10%</p>
            <p class="nivo0" style="top:300px;">0%</p>
            <p class="nivo" style="top:400px;">-10%</p>
            <p class="nivo" style="top:500px;">-20%</p>
            <p class="oznaka" style="top:550px;">-25% и мање</p>
        </div>
        ';


  $out1='<div id="main_chart"></div>';

  $fout = '<div class="has-inner-drawer">';
  if($type=='outcome') {
  $fout .= '
            <div id="docDrawer" class="drawer drawer-inside drawer-right dw-xs-6 dw-sm-6 dw-md-6 fold slide" aria-labelledby="docDrawer">
                <div class="drawer-controls">
                    <a href="#docDrawer" data-toggle="drawer" href="#docDrawer" aria-foldedopen="false" aria-controls="docDrawer" class="btn btn-default">Легенда</i></a>
                </div>
                <div class="drawer-content drawer-content-scrollable">'.$drawer.'</div>
            </div>';
  }

  $fout .='<div id="chart_wrapper">' . $out . '<main class="bmd-layout-content">'.$out1.'</main></div>';
  $fout .='</div>';
  return $fout;
}

