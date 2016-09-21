<?php
/**
 * @file
 * Uses the Tripal Download API to download genotypes from the genotype matrix.
 */

/**
 * Implements hook_register_trpdownload_type.
 */
function nd_genotypes_register_trpdownload_type() {
  $types = array();

  $types['genotype_matrix_csv'] = array(
    'type_name' => 'Genotype File',
    'format' => 'Comma-separated Values',
    'functions' => array(
      'generate_file' => 'trpdownload_genotype_matrix_download_generate_csv_file',
      'get_filename' => 'trpdownload_genotype_matrix_download_get_filename',
      'get_file_suffix' => 'trpdownload_genotype_matrix_download_get_csv_suffix',
    ),
  );

  return $types;
}

/**
 * Generates a file for download in the specified format.
 *
 * @param $variables
 *   An associative array of parameters including:
 *     - q: all the query paramters.
 *     - site_safe_name: a sanitized version of your site name for use in variables & filenames.
 *     - type_info: an array of info for the download type.
 *     - suffix: the file format suffix.
 *     - filename: the filename of the file to generate not including path.
 *     - fullpath: the full path and filename of the file to generate.
 *     - format_name: a human-readable description of the format.
 * @param $job_id
 *   The ID of the tripal job executing this function ;-).
 */
function trpdownload_genotype_matrix_download_generate_csv_file($variables, $job_id = NULL) {

  // First, process the query parameters passed to the download to determine the query.
  nd_genotypes_process_query_parameters($variables);

  // Create the file and ready it for writting to.
  $filepath = variable_get('trpdownload_fullpath', '') . $variables['filename'];
  drush_print("File: " . $filepath);
  $FILE = fopen($filepath, 'w') or die ('Unable to create file to write to');

  // Write the header to the file.
  $header = array();
  foreach ($variables['header'] as $h) {
    $header[] = $h['data'];
  }
  fputcsv($FILE, $header);

  $query = implode(' ',$variables['query']['sql']);

  // Determine the total number of lines resulting from the query
  // for tracking progress.
  /*
  $count_query = preg_replace('/SELECT.*FROM/s', 'SELECT count(*) as num_lines FROM', $query);
  $total_lines = chado_query($count_query, $variables['query']['args'])->fetchField();
  drush_print('Total Lines: '.$total_lines);
  */

  $resource = chado_query($query, $variables['query']['args']);
  $row = array();
  $current_variant = NULL;
  $cur_line = 0;
  foreach ($resource as $r) {
    $cur_line++;

    // Output the progress.
    /*
    $cur_line++;
    $percent = $cur_line/$total_lines * 100;
    if ($percent%5 == 0) {
      drush_print(round($percent,2).'% Complete.');
      db_query('UPDATE {tripal_jobs} SET progress=:percent WHERE job_id=:id',
        array(':percent' => round($percent), ':id' => $job_id));
    }
    */

    // If this is not the variant we were working on... then we must be done the last one!
    // Print and move on ;-).
    if ($r->variant_id != $current_variant) {

      // Print the last row.
      if (!empty($row)) {
        fputcsv($FILE, $row);
      }

      // Start this row.
      $row = $variables['template_row'];
      $current_variant = $r->variant_id;
    }

    // Add the current genotype to the row.
    $row['variant_name'] = $r->variant_name;
    $row['srcfeature_name'] = $r->srcfeature_name;
    $row['fmin'] = $r->fmin;
    $row['fmax'] = $r->fmax;
    $row[$r->germplasm_id] = $r->consensus_allele;

  }

  fclose($FILE);
}

/**
 * Summarize the download before generating it.
 */
function trpdownload_genotype_matrix_download_summarize_download($vars) {
  $output = '<ul>';

  $q = $vars['download_args']['q'];

  if (isset($q['variant_type'])) {
    $type_name = chado_query('SELECT name FROM {cvterm} WHERE cvterm_id=:id',
      array(':id' => $q['variant_type']))->fetchField();
    $output .= '<li>Variant Type: ' . $type_name . '.</li>';
  }
  else {
    $output .= '<li>All Variant Types.</li>';
  }
  if (isset($q['germplasm'])) {
    $output .= '<li>' . sizeof($q['germplasm']) . ' germplasm.</li>';
  }
  if (isset($q['variant_name'])) {
    $output .= '<li>' . sizeof($q['variant_name']) . ' variants.</li>';
  }
  if (isset($q['seq_range'])) {
    $str = '';
    if (isset($q['seq_range']['start'])) {
      $str = 'From '.$q['seq_range']['start']['backbone'];
      if (isset($q['seq_range']['start']['pos'])) {
        $str .= ':'.$q['seq_range']['start']['pos'];
      }
    }
    else {
      $str = 'From the beginning';
    }
    if (isset($q['seq_range']['end'])) {
      $str .= ' to '.$q['seq_range']['end']['backbone'];
      if (isset($q['seq_range']['end']['pos'])) {
        $str .= ':'.$q['seq_range']['end']['pos'];
      }
    }
    else {
      $str .= ' to the end';
    }
    $output .= '<li>' . $str . '.</li>';
  }

  $output .= '</ul>';
  return $output;
}

/**
 * Generate a readable and unique filename for the file to be generated.
 */
function trpdownload_genotype_matrix_download_get_filename($vars) {
  $filename = $vars['download_args']['safe_site_name'] .'.genotypes.' . date('YMj-his');
  return $filename;
}

/**
 * Determine the file suffix for the file to be generated.
 */
function trpdownload_genotype_matrix_download_get_csv_suffix($vars) {
  return 'csv';
}