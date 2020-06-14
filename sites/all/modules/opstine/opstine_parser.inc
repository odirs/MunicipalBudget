<?php

/** Main function called from form submit
 * @param $report_year
 * @param $municipality_id
 * @param $report_type
 * @param $file_uri
 */

require_once __DIR__ . '/vendor/autoload.php';

use PhpOffice\PhpSpreadsheet\IOFactory;

function opstine_parser_main($report_year, $municipality_id, $report_type, $file_uri){
  $path = drupal_realpath($file_uri);
  $parts = pathinfo($path);
  $extension = $parts['extension'];
  $municipality_t=taxonomy_term_load($municipality_id);
  $municipality_name=$municipality_t->name;
  $budget_data = opstine_get_budget_data($file_uri, $extension, $report_type);
  if (count($budget_data["suki"])==0) return 0;
  $budget_array =[
    "year" => $report_year,
    "municipality_id" => $municipality_id,
    "municipality_name" => $municipality_name,
    "report_type" => $report_type,
    "budget_data" => $budget_data["suki"],
  ];
  $budget_array_cp =[
    "year" => $report_year,
    "municipality_id" => $municipality_id,
    "municipality_name" => $municipality_name,
    "report_type" => $report_type,
    "budget_data" => $budget_data["cp"],
  ];

  // save bubble data
  $json_data = json_encode($budget_array);
  $file_destination = "public://json_for_mongo/".$municipality_id."_".$report_year."_".$report_type.".json";
  file_put_contents($file_destination, $json_data);

  // cp bubble data
  $json_data = json_encode($budget_array_cp);
  $file_destination = "public://json_for_mongo/".$municipality_id."_".$report_year."_".$report_type."_cp.json";
  file_put_contents($file_destination, $json_data);

  // save treemap data
  $json_data_treemap = json_encode($budget_data["treemap"]);
  $file_destination = "public://json_for_mongo/".$municipality_id."_".$report_year."_".$report_type."_treemap.json";
  file_put_contents($file_destination, $json_data_treemap);

  //todo??
  opstine_send_to_mongo($json_data);

  return 1;
}

//todo include other parsers for other file types if needed
function opstine_get_budget_data ($file_uri = null, $file_type = "csv", $report_type){
    $json_data = "";
    switch ($file_type) {
        case 'csv':
            $json_data = opstine_parse_budget_file_csv ($file_uri, $report_type);
            break;
        case 'xls':
        case 'xlsx':
          $json_data = opstine_parse_budget_file_excel ($file_uri, $report_type);
          break;
    }
    return $json_data;
}

/** extracts data from csv, checks structure
 * @param null $raw_data
 * @return array containing budget data
 */
function opstine_parse_budget_file_csv ($file_uri = '', $report_type='outcome'){
    $data_array = [];
    //-- map scv to array
    $csv_array = array_map('str_getcsv', file($file_uri));
    if ($csv_array[0][0] === "ID" && $csv_array[0][2] === "Iznos"){
        array_shift($csv_array);
        foreach ($csv_array as $line) {
            $data_array[$line[0]] = $line[2];
        }
    } else {
        drupal_set_message("Proveri da li podaci imaju odgovarajucu strukturu: ".json_encode($data_array));
    }
    return $data_array;
}

function opstine_parse_budget_file_excel($file_uri = "", $report_type) {
  //$sheetnames = ['Data Sheet #1','Data Sheet #3'];
  //$reader->setLoadSheetsOnly($sheetnames);
    $parsed_json_arr = [];
  try {
    $filet=drupal_realpath($file_uri);
    $inputFileType = IOFactory::identify(drupal_realpath($filet));
    $reader = IOFactory::createReader($inputFileType);
    $reader->setReadDataOnly(true);
    $worksheetData = $reader->listWorksheetInfo($filet);

    $good_income=array('Класа','Назив_класе','Категорија','Назив_категорије','Група','Назив_групе','Економска_класа','Назив_економске_класе','Средства_из_буџета','Средства_из_cопствених_извора','Средства_из_осталих_извора');
    $good_outcome=array('Раздео','Раздео_назив','Глава','Назив_главе','Програм','Број_програма','Назив_програма','Програмска_класификација','Назив_програмске_класификације','Функција_класе','Назив_функције','Број_позиције','Економска_класификација','Назив_економске_класификације','Средства_из_буџета','Средства_из_сопствених_извора','Средства_из_осталих_извора');

    foreach ($worksheetData as $worksheet) {
      $sheetName = $worksheet['worksheetName'];
      //echo "<h4>$sheetName</h4>";
      /**  Load $inputFileName to a Spreadsheet Object  **/
      $reader->setLoadSheetsOnly($sheetName);
      $spreadsheet = $reader->load($filet);

      $worksheet = $spreadsheet->getActiveSheet();
      $worksheet_arr=$worksheet->toArray();
      //$parsed_json_arr = opstine_worksheet_data_packer ($worksheet_arr);
      $json_arr=array();
      $titles=$worksheet_arr[0];

      $title_check=0;
      foreach ($titles as $title){
        if ($report_type=='income'){
          if(in_array(trim($title),$good_income)) $title_check++;
        } else {
          if(in_array(trim($title),$good_outcome)) $title_check++;
        }
      }

      if ($report_type=='income') {
        if (count($good_income) != $title_check) {
          drupal_set_message('Broj i/ili naziv kolona u tabeli prihoda ne odgovara zadatom obrascu', 'error');
          $output=array();
          $output["suki"]=array();
          return $output;
        }
      } else {
        if (count($good_outcome) != $title_check){
          drupal_set_message('Broj i/ili naziv kolona u tabeli rashoda ne odgovara zadatom obrascu', 'error');
          $output=array();
          $output["suki"]=array();
          return $output;
        }
      }

      for($i=1; $i<count($worksheet_arr); $i++){
        if($i==1000) break; //not more than 1000 lines
        $tmp_arr=array();

        for ($j=0; $j<count($titles);$j++){
          $input=trim($worksheet_arr[$i][$j]);
          $output = preg_replace('/\s+/', ' ',$input);
          $tmp_arr[$titles[$j]]=$output;
        }
        $json_arr[]=$tmp_arr;
      }
      $suki_json=opstine_worksheet_data_packer_suki($json_arr, $report_type);
      //$treemap_json = opstine_treemap_data_packer($json_arr, $report_type);
    }

  } catch (\PhpOffice\PhpSpreadsheet\Reader\Exception $e) {
    drupal_set_message('Greška u čitanju: ' . $e, 'error');
  }

  //return $parsed_json_arr;
    $output=[];
    $output["suki"] =  $suki_json[0];
    $output["cp"] =  $suki_json[1];
    //$output["treemap"] =  $treemap_json;
  return $output;
}

function opstine_program_mapper($sifra_programa){
    $program["1101"] = ["program_number" =>  1,"program_name" => "СТАНОВАЊЕ, УРБАНИЗАМ И ПРОСТОРНО ПЛАНИРАЊЕ","program_id" => "1101","program_sector" => "Урбанизам и просторно планирање","program_activities" => []];
    $program["1102"] = ["program_number" =>  2,"program_name" => "КОМУНАЛНЕ ДЕЛАТНОСТИ","program_id" => "1102","program_sector" => "Урбанизам и просторно планирање","program_activities" => []];
    $program["1501"] = ["program_number" =>  3,"program_name" => "ЛОКАЛНИ ЕКОНОМСКИ РАЗВОЈ","program_id" => "1501","program_sector" => "Економска и развојна политика","program_activities" => []];
    $program["1502"] = ["program_number" =>  4,"program_name" => "РАЗВОЈ ТУРИЗМА","program_id" => "1502","program_sector" => "Економска и развојна политика","program_activities" => []];
    $program["0101"] = ["program_number" =>  5,"program_name" => "ПОЉОПРИВРЕДА И РУРАЛНИ РАЗВОЈ","program_id" => "0101","program_sector" => "Пољопривреда и рурални развој","program_activities" => []];
    $program["0401"] = ["program_number" =>  6,"program_name" => "ЗАШТИТА ЖИВОТНЕ СРЕДИНЕ","program_id" => "0401","program_sector" => "Заштита животне средине","program_activities" => []];
    $program["0701"] = ["program_number" =>  7,"program_name" => "ОРГАНИЗАЦИЈА САОБРАЋАЈА И САОБРАЋАЈНА ИНФРАСТРУКТУРА","program_id" => "0701","program_sector" => "Саобраћај и саобраћајна инфраструктура","program_activities" => []];
    $program["2001"] = ["program_number" =>  8,"program_name" => "ПРЕДШКОЛСКО ВАСПИТАЊЕ И ОБРАЗОВАЊЕ","program_id" => "2001","program_sector" => "Образовање","program_activities" => []];
    $program["2002"] = ["program_number" =>  9,"program_name" => "ОСНОВНО ОБРАЗОВАЊЕ И ВАСПИТАЊЕ","program_id" => "2002","program_sector" => "Образовање","program_activities" => []];
    $program["2003"] = ["program_number" => 10,"program_name" => "СРЕДЊЕ ОБРАЗОВАЊЕ И ВАСПИТАЊЕ","program_id" => "2003","program_sector" => "Образовање","program_activities" => []];
    $program["0901"] = ["program_number" => 11,"program_name" => "СОЦИЈАЛНА И ДЕЧИЈА ЗАШТИТА","program_id" => "0901","program_sector" => "Социјална заштита","program_activities" => []];
    $program["1801"] = ["program_number" => 12,"program_name" => "ЗДРАВСТВЕНА ЗАШТИТА","program_id" => "1801","program_sector" => "Здравствена заштита","program_activities" => []];
    $program["1201"] = ["program_number" => 13,"program_name" => "РАЗВОЈ КУЛТУРЕ И ИНФОРМИСАЊА","program_id" => "1201","program_sector" => "Култура, комуникације и медији","program_activities" => []];
    $program["1301"] = ["program_number" => 14,"program_name" => "РАЗВОЈ СПОРТА И ОМЛАДИНЕ","program_id" => "1301","program_sector" => "Спорт и омладина","program_activities" => []];
    $program["0602"] = ["program_number" => 15,"program_name" => "ОПШТЕ УСЛУГЕ ЛОКАЛНЕ САМОУПРАВЕ","program_id" => "0602","program_sector" => "Опште услуге јавне управе","program_activities" => []];
    $program["2101"] = ["program_number" => 16,"program_name" => "ПОЛИТИЧКИ СИСТЕМ ЛОКАЛНЕ САМОУПРАВЕ","program_id" => "2101","program_sector" => "Политички систем","program_activities" => []];
    $program["0501"] = ["program_number" => 17,"program_name" => "ЕНЕРГЕТСКА ЕФИКАСНОСТ И ОБНОВЉИВИ ИЗВОРИ ЕНЕРГИЈЕ","program_id" => "0501","program_sector" => "Енергетика","program_activities" => []];

    if(!isset($program[$sifra_programa])){
      return array();
    }
    return $program[$sifra_programa];
}

function opstine_treemap_data_packer($iarray, $report_type){
    $tremap = array();
    if($report_type=='outcome') {
        $i = 1;
        foreach ($iarray as $item) {
            // treemap data
            $tremap_temp = new stdClass();
            $tremap_temp -> key = $i."_".$item['Назив_економске_класификације'];
            $tremap_temp -> prog_id = $item['Назив_програма'];
            $tremap_temp -> akt_id = $item['Назив_програмске_класификације'];
            $tremap_temp -> value = $item['Укупна_јавна_средства'];

            $tremap[] = $tremap_temp;
            $i++;
        }
    } elseif ($report_type=='outcome'){

    }
    return $tremap;
}

function opstine_worksheet_data_packer_suki($iarray, $report_type){
  if($report_type=='outcome') {
    //parsing outcome
    $oa = array();
    $razdeli= array();
    $programi= array();
    $aktivnosti= array();
    $ek= array();

    foreach ($iarray as $item) {
      $razdeli[$item['Раздео']] = $item['Раздео_назив'];
      $programi[$item['Програм']] = $item['Назив_програма'];
      $aktivnosti[$item['Програмска_класификација']] = $item['Назив_програмске_класификације'];
      $ek[$item['Економска_класификација']] = $item['Назив_економске_класификације'];

      $sredstva_iz_budzeta=0;
      if(isset($item['Средства_из_буџета_'])){
        if($item['Средства_из_буџета_']!='' && $item['Средства_из_буџета_']!=null) $sredstva_iz_budzeta=floatval($item['Средства_из_буџета_']);
      } else {
        if(isset($item['Средства_из_буџета'])){
          if($item['Средства_из_буџета']!='' && $item['Средства_из_буџета']!=null) $sredstva_iz_budzeta=floatval($item['Средства_из_буџета']);
        }
      }
      $sopstvena_sredstva=0;
      if(isset($item['Средства_из_сопствених_извора'])) {
        if ($item['Средства_из_сопствених_извора'] != '' && $item['Средства_из_сопствених_извора'] != NULL) {
          $sopstvena_sredstva = floatval($item['Средства_из_сопствених_извора']);
        }
      }
      $ostala_sredstva=0;
      if(isset($item['Средства_из_осталих_извора'])) {
        if ($item['Средства_из_осталих_извора'] != '' && $item['Средства_из_осталих_извора'] != NULL) {
          $ostala_sredstva = floatval($item['Средства_из_осталих_извора']);
        }
      }
      $ukupna_sredstva=0;
      if(isset($item['Укупна_јавна_средства'])) {
        if ($item['Укупна_јавна_средства'] != '' && $item['Укупна_јавна_средства'] != NULL) {
          $ukupna_sredstva = floatval($item['Укупна_јавна_средства']);
        }
      }

      if (isset($oa[$item['Раздео']][$item['Програм']][$item['Програмска_класификација']][$item['Економска_класификација']])) {
        $oa[$item['Раздео']][$item['Програм']][$item['Програмска_класификација']][$item['Економска_класификација']] =
          [
            $oa[$item['Раздео']][$item['Програм']][$item['Програмска_класификација']][$item['Економска_класификација']][0] + $sredstva_iz_budzeta,
            $oa[$item['Раздео']][$item['Програм']][$item['Програмска_класификација']][$item['Економска_класификација']][1] + $sopstvena_sredstva,
            $oa[$item['Раздео']][$item['Програм']][$item['Програмска_класификација']][$item['Економска_класификација']][2] + $ostala_sredstva,
            $oa[$item['Раздео']][$item['Програм']][$item['Програмска_класификација']][$item['Економска_класификација']][3] + $ukupna_sredstva
          ];
      } else {
        $oa[$item['Раздео']][$item['Програм']][$item['Програмска_класификација']][$item['Економска_класификација']] =
          [
            $sredstva_iz_budzeta,
            $sopstvena_sredstva,
            $ostala_sredstva,
            $ukupna_sredstva
          ];
      }
    }
    if (count($ek) < 2) {
      return [];
    }
    $o_json = [];
    $o_json_cp = [];
    foreach ($oa as $ork => $orv) {
      $razdel = new stdClass();
      $razdel->razdel_number = $ork;
      $razdel->razdel_name = $razdeli[$ork];
      $razdel_total = 0;
      $razdel_cp = new stdClass();
      $razdel_cp->name=$razdeli[$ork];
      $razdel_cp->id=$ork;
      $razdel_cp->type='razdel';
      foreach ($orv as $opk => $opv) {
        $program = new stdClass();
        $program->program_id = $opk;
        $program->program_name = $programi[$opk];
        $program_total = 0;
        $program_cp = new stdClass();
        $program_cp->id = $opk;
        $program_cp->name = $programi[$opk];
        $program_cp->type = 'program';
        foreach ($opv as $oak => $oav) {
          $activity = new stdClass();
          $activity->activity_id = $oak;
          $activity->activity_name = $aktivnosti[$oak];
          $activity_total = 0;
          $activity_cp = new stdClass();
          $activity_cp->id = $oak;
          $activity_cp->name = $aktivnosti[$oak];
          $activity_cp->type = 'aktivnost';
          foreach ($oav as $oek => $oev) {
            $economic_classification = new stdClass();
            $economic_classification->ek_number = $oek;
            $economic_classification->ek_name = $ek[$oek];
            $economic_classification->sources = [$oev[0], $oev[1], $oev[2]];
            $economic_classification->total = $oev[3];
            $activity->lines[] = $economic_classification;
            $activity_total += $oev[3];

            $economic_classification_cp = new stdClass();
            $economic_classification_cp->id = $oek;
            $economic_classification_cp->type = 'economic_classification';
            $economic_classification_cp->name = $ek[$oek];
            $economic_classification_cp->s1=$oev[0];
            $economic_classification_cp->s2=$oev[1];
            $economic_classification_cp->s3=$oev[2];
            $economic_classification_cp->total = $oev[3];
            $economic_classification_cp->size = $oev[3];
            $activity_cp->children[] = $economic_classification_cp;
          }
          $activity->total = $activity_total;
          $activity_cp->total = $activity_total;
          $program->program_activities[] = $activity;
          $program_total += $activity_total;
          $program_cp->children[] = $activity_cp;
        }
        $program->total = $program_total;
        $program_cp->total = $program_total;
        $razdel->razdel_programs[] = $program;
        $razdel_total += $program_total;
        $razdel_cp->children[] = $program_cp;
      }
      $razdel->total = $razdel_total;
      $razdel_cp->total = $razdel_total;
      $o_json[] = $razdel;
      $o_json_cp[] = $razdel_cp;
    }
  } else if($report_type=='income') {
    //parsing income
    $oa = array();
    $razdeli= array();
    $programi= array();
    $aktivnosti= array();
    $ek= array();

    foreach ($iarray as $item) {
      $razdeli[$item['Класа']] = $item['Назив_класе'];
      $programi[$item['Категорија']] = $item['Назив_категорије'];
      $aktivnosti[$item['Група']] = $item['Назив_групе'];
      $ek[$item['Економска_класа']] = $item['Назив_економске_класе'];

      $sredstva_iz_budzeta=0;
      if(isset($item['Средства_из_буџета'])){
        if($item['Средства_из_буџета']!='' && $item['Средства_из_буџета']!=null) $sredstva_iz_budzeta=floatval($item['Средства_из_буџета']);
      }
      $sopstvena_sredstva=0;
      if(isset($item['Средства_из_сопствених_извора'])) {
        if ($item['Средства_из_сопствених_извора'] != '' && $item['Средства_из_сопствених_извора'] != NULL) {
          $sopstvena_sredstva = floatval($item['Средства_из_сопствених_извора']);
        }
      }
      $ostala_sredstva=0;
      if(isset($item['Средства_из_осталих_извора'])) {
        if ($item['Средства_из_осталих_извора'] != '' && $item['Средства_из_осталих_извора'] != NULL) {
          $ostala_sredstva = floatval($item['Средства_из_осталих_извора']);
        }
      }

      if (isset($oa[$item['Класа']][$item['Категорија']][$item['Група']][$item['Економска_класа']])) {
        $oa[$item['Класа']][$item['Категорија']][$item['Група']][$item['Економска_класа']] =
          [
            $oa[$item['Класа']][$item['Категорија']][$item['Група']][$item['Економска_класа']][0] + $sredstva_iz_budzeta,
            $oa[$item['Класа']][$item['Категорија']][$item['Група']][$item['Економска_класа']][1] + $sopstvena_sredstva,
            $oa[$item['Класа']][$item['Категорија']][$item['Група']][$item['Економска_класа']][2] + $ostala_sredstva,
            $oa[$item['Класа']][$item['Категорија']][$item['Група']][$item['Економска_класа']][3] + ($sredstva_iz_budzeta+$sopstvena_sredstva+$ostala_sredstva)
          ];
      } else {
        $oa[$item['Класа']][$item['Категорија']][$item['Група']][$item['Економска_класа']] =
          [
            $sredstva_iz_budzeta,
            $sopstvena_sredstva,
            $ostala_sredstva,
            ($sredstva_iz_budzeta+$sopstvena_sredstva+$ostala_sredstva)
          ];
      }
    }
    if (count($ek) < 2) {
      return [];
    }

    $o_json = [];
    foreach ($oa as $ork => $orv) {
      $razdel = new stdClass();
      $razdel->razdel_number = $ork;
      $razdel->razdel_name = $razdeli[$ork];
      $razdel_total = 0;
      $razdel_cp = new stdClass();
      $razdel_cp->name=$razdeli[$ork];
      $razdel_cp->id=$ork;
      $razdel_cp->type='klasa';
      foreach ($orv as $opk => $opv) {
        $program = new stdClass();
        $program->program_id = $opk;
        $program->program_name = $programi[$opk];
        $program_total = 0;
        $program_cp = new stdClass();
        $program_cp->id = $opk;
        $program_cp->name = $programi[$opk];
        $program_cp->type = 'kategorija';
        foreach ($opv as $oak => $oav) {
          $activity = new stdClass();
          $activity->activity_id = $oak;
          $activity->activity_name = $aktivnosti[$oak];
          $activity_total = 0;
          $activity_cp = new stdClass();
          $activity_cp->id = $oak;
          $activity_cp->name = $aktivnosti[$oak];
          $activity_cp->type = 'grupa';
          foreach ($oav as $oek => $oev) {
            $economic_classification = new stdClass();
            $economic_classification->ek_number = $oek;
            $economic_classification->ek_name = $ek[$oek];
            $economic_classification->sources = [$oev[0], $oev[1], $oev[2]];
            $economic_classification->total = $oev[3];
            $activity->lines[] = $economic_classification;
            $activity_total += $oev[3];
            $economic_classification_cp = new stdClass();
            $economic_classification_cp->id = $oek;
            $economic_classification_cp->type = "economic_classification";
            $economic_classification_cp->name = $ek[$oek];
            $economic_classification_cp->s1 = $oev[0];
            $economic_classification_cp->s2 = $oev[1];
            $economic_classification_cp->s3 = $oev[2];
            $economic_classification_cp->total = $oev[3];
            $economic_classification_cp->size = $oev[3];
            $activity_cp->children[] = $economic_classification_cp;
          }
          $activity->total = $activity_total;
          $activity_cp->total = $activity_total;
          $program->program_activities[] = $activity;
          $program_total += $activity_total;
          $program_cp->children[] = $activity_cp;
        }
        $program->total = $program_total;
        $program_cp->total = $program_total;
        $razdel->razdel_programs[] = $program;
        $razdel_total += $program_total;
        $razdel_cp->children[] = $program_cp;
      }
      $razdel->total = $razdel_total;
      $razdel_cp->total = $razdel_total;
      $o_json[] = $razdel;
      $o_json_cp[] = $razdel_cp;
    }
  } else {
    $o_json=array();
    $o_json_cp=array();
  }
  return array($o_json, $o_json_cp);
}

function opstine_worksheet_data_packer ($worksheet_arr){
    $program_array = [];
    //Програм 4	Број_програма 5	Назив_програма 6 Програмска_класификација 7 Назив_програмске_класификације 8 Функција_класе	9 Назив_функције 10
    $val_arr = [];
    $total_arr = [];
    $titles=$worksheet_arr[0];
    $s_p = array_search("Програм", $titles);
    $b_p = array_search("Број_програма", $titles);
    $n_p = array_search("Назив_програма", $titles);
    $scp = array_search("Назив_функције", $titles);
    $s_a = array_search("Програмска_класификација", $titles);
    $n_a = array_search("Назив_програмске_класификације", $titles);
    //$val = array_search("Укупна_јавна_средства", $titles);
    $col_number = count($titles);
/*    $s_p = 4;
    $b_p = 5;
    $n_p = 6;
    $scp = 10;
    $s_a = 7;
    $n_a = 8;*/
    $val = $col_number-1;

    for($i=1; $i<count($worksheet_arr); $i++){
        $sifra_programa = trim($worksheet_arr[$i][$s_p]);
        $sifra_aktivnosti = trim($worksheet_arr[$i][$s_a]);
        $naziv_aktivnosti = preg_replace('/\s+/', ' ',trim($worksheet_arr[$i][$n_a]));
        //for control purposes if needed later on
        $br_programa = preg_replace('/\s+/', ' ',trim($worksheet_arr[$i][$b_p]));
        $naziv_programa = preg_replace('/\s+/', ' ',trim($worksheet_arr[$i][$n_p]));
        $sektor_programa = preg_replace('/\s+/', ' ',trim($worksheet_arr[$i][$scp]));

        $value = $worksheet_arr[$i][$val];
        isset($val_arr[$sifra_programa][$sifra_aktivnosti]["value"]) ? $val_arr[$sifra_programa][$sifra_aktivnosti]["value"] += $value : $val_arr[$sifra_programa][$sifra_aktivnosti]["value"] = $value;
               $val_arr[$sifra_programa][$sifra_aktivnosti]["name"] = $naziv_aktivnosti;
        isset($total_arr[$sifra_programa]) ? $total_arr[$sifra_programa] += $value : $total_arr[$sifra_programa] = $value;
    }

    foreach ($total_arr as $key => $value) {
        $item = opstine_program_mapper($key);
        if(count($item)>0){
        $item_act = [];
        foreach ($val_arr[$key] as $act_key => $activity) {
          $tmp_act = [];
          $tmp_act["activity_id"] = $act_key;
          $tmp_act["activity_name"] = $activity["name"];
          $tmp_act["activity_sum"] = $activity["value"];
          $item_act[] = $tmp_act;
        }
        $item["program_activities"] = array_values($item_act);
        $program_array[] = $item;
      }
    }
    $full_json_arr["programs"] = array_values($program_array);
    return $full_json_arr;
}


function opstine_visualization_data_from_file($year1, $year2) {
    $file_folder = "public://json_for_mongo/";
    $files = array_slice(scandir($file_folder), 2);
    $all_data_array = [];
    foreach ($files as $filename) {
        $data =  json_decode(file_get_contents($file_folder.$filename));
        $all_data_array[$data->municipality_id][$data->report_type][$data->year]=[$data->budget_data];
    }
    $json_data = json_encode($all_data_array);
    $file_destination = "public://opstine_data/all_data_cache.json";
    file_put_contents($file_destination, $json_data);

    $source_model = '
    {"year":"2012","municipality_id":"123","report_type":"income","budget_data":{"7_a1":"4632.399797","7_a2":"4189.670593","7_a3":"1575.845397","7_a4":"3043.579285","7_a5":"2985.446301","7_b1":"9352.18206","7_b2":"1990.916268","7_c1":"4209.793283","7_c2":"757.7836223","7_c3":"5455.137179","7_c4":"9888.009787","7_c5":"7011.541128","7_c6":"9731.741984","8_a1":"6014.566379","9_a1":"8545.361676","N_1":"5710.120232","N_2":"6193.365529"}}
    ';
    //drupal_set_message($json_data);
    opstine_visualization_data_render($all_data_array, 2012, 2013);
   return $all_data_array;
}

//todo
function opstine_visualization_data_render($data_array = [], $year1 = "", $year2 = "", $type = "income") {

    $bubble_array = [];

    foreach ($data_array as $tid => $value){
        $municipaliy_array = [];
        $term = taxonomy_term_load($tid);
        $name = $term->name;
        $municipaliy_array["glava"] = "1.0";
        $municipaliy_array["nglava"] = $name;
        $municipaliy_array["razdeo"] = "1";
        $municipaliy_array["nrazdeo"] = $name;
/*      $values_year1 = array_values($value["income"][$year1]);
        $values_year2 = array_values($value["income"][$year2]);*/
        $values_year1 = array_values((array) $value[$type][$year1][0]);
        $values_year1 = array_map('round', $values_year1);
        $values_year2 = array_values((array) $value[$type][$year2][0]);
        $values_year2 = array_map('round', $values_year2);
        $municipaliy_array["u".$year1] = array_sum($values_year1);
        $municipaliy_array["u".$year2] = array_sum($values_year2);
        $municipaliy_array["b".$year1] = $values_year1;
        $municipaliy_array["b".$year2] = $values_year2;

        $bubble_array[] = $municipaliy_array;
    }

    $model = "
    {'glava': '1.0', 'nglava': 'НАРОДНА СКУПШТИНА', 'razdeo': '1', 'nrazdeo': 'НАРОДНА СКУПШТИНА', 'u2018': 638139000, 'u2019': 669104000, 'b2018': [638139000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], 'b2019': [669104000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]}

    ";

    $json_data = json_encode($bubble_array);
    $file_destination = "public://opstine_data/".$type.".json";
    file_put_contents($file_destination, $json_data);
}

/**
 * @param $json_data
 */
//todo
function opstine_send_to_mongo($json_data) {

}

//{"fid":"3","uid":"6","filename":"ada_rashodi.csv","uri":"public:\/\/dokumenti\/ada_rashodi_0.csv","filemime":"text\/csv","filesize":"1262","status":"0","timestamp":"1579691994","type":"undefined","rdf_mapping":[],"metadata":[]}

// ODAVDE POCINJE STRANA KOJA KADA SE POZOVE PRODJE KROZ SVE SUBMITE OPSTINA I GENERISE POKRENJE GENERISANJE JSONA.
// CISTO DA NE UPLOADUJEMO STALNO DOKUMENT. LAKSE ZA TESTIRANJE

function opstine_report_manual_parser(){
  return theme('opstine_report_manual_parser',array());
}
function theme_opstine_report_manual_parser(){
  $out='';
  $submited_reports = db_select('opstine_reports', 'r')->fields('r', array())->execute();
  $all_reports = $submited_reports->fetchAll();
  if (count($all_reports)){
    $out.='<table><thead>';
    $out.='<tr><th>Godina</th><th>Opština</th><th>Prihodi</th><th>Rashodi</th></tr></thead><tbody>';
    foreach ($all_reports as $report){
      $municipality_id = $report->municipality_id;
      $opstina_term=taxonomy_term_load($municipality_id);
      $report_year=$report->report_year;
      $file_pr = file_load($report->file_income_fid);
      $file_pr_uri = $file_pr->uri;
      $file_ra = file_load($report->file_outcome_fid);
      $file_ra_uri = $file_ra->uri;

      opstine_parser_main($report_year, $municipality_id, 'income', $file_pr_uri);
      opstine_parser_main($report_year, $municipality_id, 'outcome', $file_ra_uri);
      $out.='<tr><td>'.$report_year.'</td><td>'.$opstina_term->name.'</td><td>'.$file_pr_uri.'</td><td>'.$file_ra_uri.'</td></tr>';

    }
    $out.='</tbody></table>';
  } else {
    $out.=t('There is no reports');
  }

  return $out;
}
