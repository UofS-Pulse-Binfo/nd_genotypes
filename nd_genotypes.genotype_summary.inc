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
  
  $output .= '<div class="views-widget">';
  $output .= drupal_get_form(
    'nd_genotypes_genotype_summary_filter_form',
    $_GET['stockA_organism_id'],
    $_GET['stockA_name'],
    $_GET['stockB_organism_id'],
    $_GET['stockB_name']
  );
  $output .= '</div>';
    
  if ($stockA_id && $stockB_id) {
    $output .= '<br /><h3>Comparison of Genotypes between '.$stockA.' and '.$stockB.'</h3>';
    $output .= views_embed_view('genotype_summary', 'default', $stockA_id,$stockB_id);
  }
  
  return $output;
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
        'genotype/summary',
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
        'genotype/summary',
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