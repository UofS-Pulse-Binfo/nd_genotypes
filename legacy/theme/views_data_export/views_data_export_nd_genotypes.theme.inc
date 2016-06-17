<?php

/**
 * @file
 * Preprocess functions related to views data export formats/plugins.
 */

/**
 * @section
 * Format: Genotype Table (CSV)
 */

/**
 * Genotype_table: Header Preprocess
 */
function nd_genotypes_preprocess_views_data_export_genotype_table_header(&$vars) {
  _views_data_export_header_shared_preprocess($vars);

  // Add genotype_germplasm fields that have data.
  if (isset($vars['view']->filter['add_germplasm_genotype'])) {
    foreach ($vars['view']->filter['add_germplasm_genotype']->value['existing'] as $k => $stock) {
      $key = (!empty($k)) ? 'germplasm_genotype_' . $k : 'germplasm_genotype';
      $vars['header'][$key] = $stock['stock_name'];
    }
  }

  // Special handling when quoted values are involved.
  if ($vars['options']['quote']) {
    $wrap = '"';
    $replace_value = '""';
  }
  else {
    $wrap = '';
    $replace_value = '';
  }

  // Format header values.
  foreach ($vars['header'] as $key => $value) {
    $output = decode_entities($value);
    $output = (empty($vars['options']['keep_html'])) ? strip_tags($output) : $output;
    if (!empty($vars['options']['trim'])) {
      $output = trim($output);
    }
    if (!empty($vars['options']['encoding']) && function_exists('iconv')) {
      switch($vars['options']['encoding']) {
        case 'utf8_decode':
          $converted = utf8_decode($output);
          break;
        default:
          $converted = iconv("UTF-8", $vars['options']['encoding'], $output);
          break;
      }
      if ($converted !== FALSE) {
        $output = $converted;
      }
    }
    $vars['header'][$key] = $wrap . str_replace('"', $replace_value, $output) . $wrap;
  }
}

/**
 * Genotype Table: Body Preprocess
 */
function nd_genotypes_preprocess_views_data_export_genotype_table_body(&$vars) {
  _views_data_export_body_shared_preprocess($vars);

  // Add genotypes to "themed rows" which are actually pre-themed rows ;)
  // First we need to determine which germplasm genotype fields have data.
  $germplasm_genotype_fields = array();
  foreach ($vars['view']->field as $field_name => $field) {
    if (preg_match('/germplasm_genotype_*(\d*)/', $field_name, $matches)) {
      $k = (empty($matches[1])) ? 0 : $matches[1];
      if (isset($vars['view']->exposed_data['add_germplasm_genotype']['existing'][$k])) {
        $germplasm_genotype_fields[$k] = $vars['view']->exposed_data['add_germplasm_genotype']['existing'][$k];
        $germplasm_genotype_fields[$k]['field_name'] = $field_name;

      }
    }
  }

  // Add genotypes to "themed rows" which are actually pre-themed rows ;)
  // Second we need to go through the results and grab the data for each field.
  foreach ($vars['rows'] as $num => $row) {

    // Add in the marker name while we're at it...
    // @todo remove this and instead fix the view.
    if (empty($vars['themed_rows'][$num]['name'])) {
      if (isset($row->feature_feature_name)) {
        $vars['themed_rows'][$num]['name'] = $row->feature_feature_name;
      }
    }

    // Now for each germplasm genotype field with data, add in the genotypes.
    foreach ($germplasm_genotype_fields as $field_details) {

      // The query field containing the alleles has the nameing
      // pattern: stock[stock_id]_allele and contains a postgreSQL array.
      // Thus if it's set we need to extract the elements of the array and render them.
      $key = 'stock' . $field_details['stock_id'] . '_allele';
      $field_name = $field_details['field_name'];
      if (isset($row->{$key})) {
        if (preg_match('/{(.*)}/', $row->{$key}, $matches)) {
          $vars['themed_rows'][$num][$field_name] = str_replace(',','/',$matches[1]);
        }
      }
    }
  }

  // Special handling when quoted values are involved.
  if ($vars['options']['quote']) {
    $wrap = '"';
    $replace_value = '""';
  }
  else {
    $wrap = '';
    $replace_value = '';
  }

  // Format row values.
  foreach ($vars['themed_rows'] as $i => $values) {
    foreach ($values as $j => $value) {
      $output = decode_entities($value);
      $output = (empty($vars['options']['keep_html'])) ? strip_tags($output) : $output;
      if (!empty($vars['options']['trim'])) {
        $output = trim($output);
      }

      if (!empty($vars['options']['encoding']) && function_exists('iconv')) {
        switch($vars['options']['encoding']) {
          case 'utf8_decode':
            $converted = utf8_decode($output);
            break;
          default:
            $converted = iconv("UTF-8", $vars['options']['encoding'], $output);
            break;
        }
        if ($converted !== FALSE) {
          $output = $converted;
        }
      }
      if (!empty($vars['options']['replace_newlines'])) {
        $output = str_replace("\n", $vars['options']['newline_replacement'], $output);
      }
      $vars['themed_rows'][$i][$j] = $wrap . str_replace('"', $replace_value, $output) . $wrap;
    }
  }
}