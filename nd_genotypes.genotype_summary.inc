<?php

/**
 *
 */
function nd_genotypes_genotype_summary_page () {
  $output = '';
  
  $stockA = $_GET['stockA_name'];
  $stockA_id = $_GET['stockA_id'];
  $stockB = $_GET['stockB_name'];
  $stockB_id = $_GET['stockB_id'];
  
  // Render the form including defaults if available
  $form_state['values']['stockA']['organism_id'] = $_GET['stockA_organism_id'];
  $form_state['values']['stockA']['stock_name'] = $_GET['stockA_name'];
  $form_state['values']['stockB']['organism_id'] = $_GET['stockB_organism_id'];
  $form_state['values']['stockB']['stock_name'] = $_GET['stockB_name'];

  // Title
  if ($stockA_id && $stockB_id) {
    $output .= '<br /><h3>Comparison of Genotypes between '.$stockA.' and '.$stockB.'</h3>';
  }
    
  // Header
  $genotype_listing_url = nd_genotypes_genotype_comparison_view_path(
    array('id'=>$_GET['stockA_id'],'name'=>$_GET['stockA_name'],'organism_id'=>$_GET['stockA_organism_id']), 
    array('id'=>$_GET['stockB_id'],'name'=>$_GET['stockB_name'],'organism_id'=>$_GET['stockB_organism_id'])
  );
  $genotype_dynamic_listing_url = nd_genotypes_genotype_comparison_dynamic_view_path(
    array('id'=>$_GET['stockA_id'],'name'=>$_GET['stockA_name'],'organism_id'=>$_GET['stockA_organism_id']), 
    array('id'=>$_GET['stockB_id'],'name'=>$_GET['stockB_name'],'organism_id'=>$_GET['stockB_organism_id'])
  );
  $output .= "<p>By entering the name of a line/variety in the 'First Line/Variety' and 
              'Second Line/Variety' filter below you can get an idea of the number of 
              polymorphic/monomorphic loci between them. The 'All' row provides
              the number of unique polymorphic/monomorphic loci between the two
              lines/varieties.</p>
              <p>If you would like more detail including the actual allele calls for
              a set of markers, then use <a href='".$genotype_listing_url."'>Genotype Comparison</a> or if you might want to add additional lines/varieties to the comparison then use the <a href='".$genotype_dynamic_listing_url.".'>Genus-specific Genotype Comparison</a></p>";
  
  // Filters
  $output .= '<div class="views-widget">';
  $output .= drupal_get_form(
    'nd_genotypes_genotype_summary_filter_form',
    $_GET['stockA_organism_id'],
    $_GET['stockA_name'],
    $_GET['stockB_organism_id'],
    $_GET['stockB_name']
  );
  $output .= '</div>';
  
  // View  
  if ($stockA_id && $stockB_id) {
    $view_output = views_embed_view('genotype_summary', 'default', $stockA_id,$stockB_id);
    $output .= "<br>".$view_output;
    
    if (!$view_output) {
      drupal_set_message('genotype_summary view is missing.','error');
    }
  }
  
  return $output;
}

/**
 * Generate the path to the corresponding genotype comparison view based on a 
 * genotype summary page.
 *
 * @param $stock1
 *   an array containing the id (stock_id), name, organism_id for the first line/variety
 * @param $stock2
 *   an array containing the id (stock_id), name, organism_id for the second line/variety
 * @return $url
 *   The URL of an nd_genotype_comparison view for the first/second line/variety fields
 */
function nd_genotypes_genotype_comparison_view_path ($stock1, $stock2) {
  global $base_url;
  
  // Determine which of the two stocks has the smallest stock_id and create the url
  $format = "genotypes?line1%%5Borganism_id%%5D=%d&line1%%5Bstock_name%%5D=%s&line2%%5Borganism_id%%5D=%d&line2%%5Bstock_name%%5D=%s&polymorphic%%5Bfirst%%5D=<NONE>&polymorphic%%5Bsecond%%5D=<NONE>&genus=%s";
  if ($stock1['id']) {

    $previous_db = tripal_db_set_active('chado');
    $organism = db_fetch_object(db_query('SELECT genus FROM organism WHERE organism_id=%d',$stock1['organism_id']));
    tripal_db_set_active($previous_db);
    
    $params = array(
      'stock1_organism_id' => trim($stock1['organism_id']), 'stock1_name' => trim($stock1['name']),
      'stock2_organism_id' => trim($stock2['organism_id']), 'stock2_name' => trim($stock2['name']),
      $organism->genus
    );  
    $url = $base_url .'/'. vsprintf($format,$params);
  } else {
    $url = $base_url.'/genotypes';
  }
  
  return $url;
}

/**
 * Generate the path to the corresponding genotype comparison view based on a 
 * genotype summary page.
 *
 * @param $stock1
 *   an array containing the id (stock_id), name, organism_id for the first line/variety
 * @param $stock2
 *   an array containing the id (stock_id), name, organism_id for the second line/variety
 * @return $url
 *   The URL of an nd_genotype_comparison view for the first/second line/variety fields
 */
function nd_genotypes_genotype_comparison_dynamic_view_path ($stock1, $stock2) {
  global $base_url;
  
  // Determine which of the two stocks has the smallest stock_id and create the url
  $format = "genotype/%s/comparison?add_germplasm_genotype[0][stock_name]=%s&add_germplasm_genotype[0][stock_id]=%d&add_germplasm_genotype[0][organism_id]=%d&add_germplasm_genotype[1][stock_name]=%s&add_germplasm_genotype[1][stock_id]=%d&add_germplasm_genotype[1][organism_id]=%d";
  if ($stock1['id']) {

    $previous_db = tripal_db_set_active('chado');
    $organism = db_fetch_object(db_query('SELECT genus FROM organism WHERE organism_id=%d',$stock1['organism_id']));
    tripal_db_set_active($previous_db);
    
    $params = array(
      $organism->genus,
      'stock1_name' => trim($stock1['name']),'stock1_id' => trim($stock1['id']),'stock1_organism_id' => trim($stock1['organism_id']), 
      'stock2_name' => trim($stock2['name']),'stock2_id' => trim($stock2['id']),'stock2_organism_id' => trim($stock2['organism_id'])
    );  
    $url = $base_url .'/'. vsprintf($format,$params);
  } else {
    $url = $base_url.'/genotype/all/comparison';
  }
  
  return $url;
}

/**
 *
 */
function nd_genotypes_genotype_summary_filter_form ($form_state, $stockA_organism_id = NULL, $stockA_name = NULL, $stockB_organism_id = NULL, $stockB_name = NULL) {
  $form = array();
  
  $form['markup'] = array(
    '#type' => 'markup',
    '#prefix' => '<div class="views-exposed-form views-exposed-widgets clear-block">',
    '#suffix' => '</div>',
  );
  
  $form['markup']['stockA'] = array(
    '#type' => 'organism_stock',
    '#title' => 'First Line/Variety',
    '#default_value' => ($form_state['values']['stockA']) ? $form_state['values']['stockA'] : array('organism_id' => $stockA_organism_id,'stock_name' => $stockA_name),
    '#size' => 30,
    '#prefix' => '<div class="views-exposed-widget views-widget">',
    '#suffix' => '</div>'
  );

  $form['markup']['stockB'] = array(
    '#type' => 'organism_stock',
    '#title' => 'Second Line/Variety',
    '#default_value' => ($form_state['values']['stockB']) ? $form_state['values']['stockB'] : array('organism_id' => $stockB_organism_id,'stock_name' => $stockB_name),
    '#size' => 30,
    '#prefix' => '<div class="views-exposed-widget views-widget">',
    '#suffix' => '</div>'
  );
  
  $form['markup']['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Generate Summary',
    '#prefix' => '<div class="views-exposed-widget views-submit-button">',
    '#suffix' => '</div>'
  );
    
  return $form;
}

function nd_genotypes_genotype_summary_filter_form_submit (&$form, &$form_state) {
  
  $form_state['rebuild'] = true;
  
  // If they submitted a value for both stockA and stockB
  if ($form_state['values']['stockA']['stock_name'] && $form_state['values']['stockB']['stock_name']) {
    
    // Get the stock_id for each
    $stockA = tripal_stock_get_stock_by_name_identifier($form_state['values']['stockA']['stock_name'], $form_state['values']['stockA']['organism_id']);
    $stockB = tripal_stock_get_stock_by_name_identifier($form_state['values']['stockB']['stock_name'], $form_state['values']['stockB']['organism_id']);
    
    // send errors if either is missing
    if (!$stockA[0]->stock->stock_id) {
      drupal_set_message('The first line/variety you entered does not exist','error');
      return FALSE;
    } 
    if (!$stockB[0]->stock->stock_id) {
      drupal_set_message('The second line/variety you entered does not exist','error');
      return FALSE;
    }
    
    // If we get here we have both stockA and stockB stock_ids
    // make sure the smallest stock_id is stockA
    if ($stockA[0]->stock->stock_id < $stockB[0]->stock->stock_id) {
      drupal_goto(
        'genotypes/summary',
        array(
          'stockA_id'=> $stockA[0]->stock->stock_id,
          'stockA_organism_id'=> $stockA[0]->stock->organism_id->organism_id,
          'stockA_name'=> $stockA[0]->stock->name,
          'stockB_id'=> $stockB[0]->stock->stock_id,
          'stockB_organism_id'=> $stockB[0]->stock->organism_id->organism_id,
          'stockB_name'=> $stockB[0]->stock->name,
        )
      );
    } else {
      drupal_goto(
        'genotypes/summary',
        array(
          'stockB_id'=> $stockA[0]->stock->stock_id,
          'stockB_organism_id'=> $stockA[0]->stock->organism_id->organism_id,
          'stockB_name'=> $stockA[0]->stock->name,
          'stockA_id'=> $stockB[0]->stock->stock_id,
          'stockA_organism_id'=> $stockB[0]->stock->organism_id->organism_id,
          'stockA_name'=> $stockB[0]->stock->name,
        )
      );
    }
  }
  
}