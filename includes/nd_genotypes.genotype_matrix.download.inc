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

  $types['genotype_matrix_hapmap'] = array(
    'type_name' => 'Genotype HAPMAP File',
    'format' => 'Haplotype Map (HAPMAP) File',
    'functions' => array(
      'generate_file' => 'trpdownload_genotype_matrix_download_generate_hapmap_file',
      'get_filename' => 'trpdownload_genotype_matrix_download_get_filename',
      'get_file_suffix' => 'trpdownload_genotype_matrix_download_get_txt_suffix',
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
function trpdownload_genotype_matrix_download_generate_csv_file($vars, $job_id = NULL) {

  // First, process the query parameters passed to the download to determine the query.
  nd_genotypes_process_query_parameters($vars);

  // Create the file and ready it for writting to.
  $filepath = variable_get('trpdownload_fullpath', '') . $vars['filename'];
  drush_print("File: " . $filepath);
  $FILE = fopen($filepath, 'w') or die ("Unable to create file to write to.\n");


  // Write the header to the file.
  $header = array();
  foreach ($vars['header'] as $h) {
    $header[] = $h['data'];
  }
  fputcsv($FILE, $header);

  // Now, determine the query.
  // Unfortuntly we can't just use the same query as to generate the page since that is done
  // using two queries: 1 to select the variants for the page and 2 to grab the calls for
  // those variants/germplasm.
  // Instead we will use a single query to grab both variant and genotype call informtion
  // and we will optimize it by keeping each row to a single germplasm (less joins)
  // and programatically compiling it before printing.

  // With that in mind we first restrict the query based on the query parameters passed to us:
  $query_args = $vars['query_args'];
  $where = array();
  $args = array();
  $variant_where = array();
  $variant_args = array();

  // Sequence Ranges:
  // First get the arguements. There must always be a backbone and fmin even if the user
  // did not provide one.
  if (isset($query_args['start_backbone'])) {
    if (isset($query_args['start_pos'])) {
      $args[':sbackbone'] = $query_args['start_backbone'];
      $args[':sfmin'] = (int) $query_args['start_pos'];
    }
    else {
      $args[':sbackbone'] = $query_args['start_backbone'];
      $args[':sfmin'] = 1;
    }
    unset($query_args['start_backbone'], $query_args['start_pos']);
  }
  if (isset($query_args['end_backbone'])) {
    if (isset($query_args['end_pos'])) {
      $args[':ebackbone'] = $query_args['end_backbone'];
      $args[':efmin'] = (int) $query_args['end_pos'];
    }
    else {
      $args[':ebackbone'] = $query_args['end_backbone'];
      // @TODO: Switch to bigints then change this value to match the largest bigint.
      $args[':efmin'] = 2147483645;
    }
    unset($query_args['end_backbone'], $query_args['end_pos']);
  }
  // Now add the query where condition.
  if (isset($args[':sbackbone']) AND isset($args[':ebackbone'])) {
    $where[] = 'ROW(v.srcfeature_name, v.fmin) BETWEEN ROW(:sbackbone, :sfmin) AND ROW(:ebackbone,:efmin)';
  }
  elseif (isset($args[':sbackbone'])) {
    $where[] = 'ROW(v.srcfeature_name, v.fmin) >= ROW(:sbackbone, :sfmin)';
  }
  elseif (isset($args[':ebackbone'])) {
    $where[] = 'ROW(v.srcfeature_name, v.fmin) <= ROW(:ebackbone, :efmin)';
  }

  // Variant Type:
  if (isset($query_args['variant_type'])) {
    $where[] = 'variant_type=:variant_type';
    $args[':variant_type'] = $query_args['variant_type'];
    unset($query_args['variant_type']);
  }
  // Variant Name:
  $variant_name_filter = FALSE;
  if (isset($query_args['variant_name'])) {
    $where[] = "variant_name IN (VALUES('" . implode("'), ('", $query_args['variant_name']) . "'))";
    unset($query_args['variant_name']);
    $variant_name_filter = TRUE;
  }
  elseif (isset($query_args['variant_name_list'])) {
    $list_json = db_query('SELECT list FROM {ndg_matrix_variant_user_lists} WHERE list_id=:id',
      array(':id' => $query_args['variant_name_list']))->fetchField();
    $list = json_decode($list_json);
    $where[] = "variant_name IN (VALUES('" . implode("'), ('", $list) . "'))";
  }

  // So far all the filter criteria have applied to the variant list
  // so save that for use with counting.
  $variant_where = $where;
  $variant_args = $args;

  // Polymorphic:
  $polymorphic_filter = FALSE;
  $join = ' ';
  if (isset($query_args['polymorphic'])
    AND is_numeric($query_args['polymorphic']['first'])
    AND is_numeric($query_args['polymorphic']['second'])) {
        $join = "LEFT JOIN (
            SELECT a.variant_id, a.allele_call=b.allele_call as monomorphic
            FROM {mview_ndg_calls} a, {mview_ndg_calls} b
            WHERE a.variant_id=b.variant_id AND a.germplasm_id=:poly1 AND b.germplasm_id=:poly2
          ) p ON p.variant_id=v.variant_id ";
        $args[':poly1'] = $query_args['polymorphic']['first'];
        $args[':poly2'] = $query_args['polymorphic']['second'];
        $where[] = "p.monomorphic IS FALSE";
        $polymorphic_filter = TRUE;
  }

  // So far all the filter criteria have applied to the variant list
  // so save that for use with counting.
  $variant_where = $where;
  $variant_args = $args;

  // Germplasm:
  if (isset($query_args['germplasm_id']) AND !empty($query_args['germplasm_id'])) {
    $where[] = 'germplasm_id IN (:germplasm_ids)';
    $args[':germplasm_ids'] = $query_args['germplasm_id'];
    unset($query_args['germplasm_id']);
  }

  // Now determine the sort.
  if (isset($query_args['sort']) AND $query_args['sort'] == 'variant_name') {
    $order = array('variant_name', 'srcfeature_name', 'fmin');
  }
  else {
    $order = array('srcfeature_name', 'fmin');
  }

  // Finally we build the query.
  $query = '
    SELECT
      v.ndg_variants_id,
      v.variant_id,
      v.variant_name,
      v.srcfeature_name,
      v.fmin,
      v.fmax,
      c.germplasm_id,
      c.allele_call
    FROM {mview_ndg_variants} v
    LEFT JOIN {mview_ndg_calls} c USING(variant_id)
    ' . $join . '
    WHERE '. implode(' AND ', $where) .'
    ORDER BY '. implode(' ASC, ', $order) .' ASC';

  // Get an estimated number of rows.
  $count_query = 'SELECT v.variant_id FROM {mview_ndg_variants} v ' . $join;
  if (!empty($variant_where)) {
    $count_query .= ' WHERE '. implode(' AND ', $variant_where);
  }
  // @debug print "SQL: " . print_r($count_query, TRUE) . "; ARGS: " . print_r($variant_args, TRUE);
  $count_query = ndg_mview_query($vars['query_args']['partition'], $count_query, $variant_args, array('return_query' => TRUE));
  $total_rows = trpdownload_estimate_rows($count_query, $variant_args);
  print "Estimated number of rows: $total_rows.\n";


  // Now to start querying.
  // First start the transaction and create the cursor.
  print "\nStarting Export...\n";
  $cursor_name = 'download_calls';
  chado_query('BEGIN WORK');
  // @debug print_r($query); print_r($args);
  ndg_mview_query($vars['query_args']['partition'], 'DECLARE '.$cursor_name.' NO SCROLL CURSOR WITH HOLD FOR '.$query, $args);

  // Next start grabing chunks.
  $num_rows_exported = 0;
  while ($results = chado_query('FETCH FORWARD 1000 FROM '.$cursor_name)) {

    $some_results = FALSE;
    $last_percent = NULL;
    foreach ($results as $r) {
      $some_results = TRUE;

      // Our results are comming in one row per variant/germplasm combo.
      // As such we need to compile them into a row before printing to the file.
      // This is much faster then merging in the query!

      // Use the primary key of the variant mview to determine if this should be a new row.
      if (!isset($current_key) OR $current_key != $r->ndg_variants_id) {

        // Print the last row unless this is the first one.
        if (isset($row)) {
          fputcsv($FILE, $row);
          $num_rows_exported++;

          // Update progress if we've reached an increment.
          $percent = round(($num_rows_exported/$total_rows)*100);
          if ( (($percent % 5) == 0) AND ($percent != 100) AND ($percent != $last_percent)) {
            print $percent . '%...';
            db_query('UPDATE {tripal_jobs} SET progress=:percent WHERE job_id=:job_id',
              array(':percent' => $percent, ':job_id' => $job_id));
              chado_query('COMMIT WORK');
          }
          $last_percent = $percent;
        }

        // Start a new row.
        $current_key = $r->ndg_variants_id;
        $row = $vars['template_row'];
        $row['variant_name'] = $r->variant_name;
        $row['srcfeature_name'] = $r->srcfeature_name;
        $row['fmin'] = $r->fmin;
        $row['fmax'] = $r->fmax;
      }

      // Add the call to the current row.
      $row[ $r->germplasm_id ] = $r->allele_call;
    }

    if (!$some_results) { break; }
  }

  // print the last row.
  if (!empty($row)) {
    fputcsv($FILE, $row);
    $num_rows_exported++;
  }

  // Finally, complete.
  chado_query('CLOSE '.$cursor_name);
  chado_query('COMMIT WORK');
  chado_query('END');

  fclose($FILE);

  db_query('UPDATE {tripal_jobs} SET progress=:percent WHERE job_id=:job_id',
    array(':percent' => 100, ':job_id' => $job_id));
    print "100%\n";

  print "\nExport Complete ($num_rows_exported rows).\n\n";
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
function trpdownload_genotype_matrix_download_generate_hapmap_file($vars, $job_id = NULL) {

  // First, process the query parameters passed to the download to determine the query.
  nd_genotypes_process_query_parameters($vars);

  // Create the file and ready it for writting to.
  $filepath = variable_get('trpdownload_fullpath', '') . $vars['filename'];
  drush_print("File: " . $filepath);
  $FILE = fopen($filepath, 'w') or die ("Unable to create file to write to.\n");

  // @debug drush_print("VARS: ".print_r($vars, TRUE));

  // Write the header to the file.
  $header = array(
    'rs' => 'rs#',
    'alleles' => 'alleles',
    'chrom' => 'chrom',
    'pos' => 'pos',
    'strand' => 'strand',
    'assembly' => 'assembly#',
    'center' => 'center',
    'prot' => 'protLSID',
    'assay' => 'assayLSID',
    'panel' => 'panel',
    'qccode' => 'QCcode');

  foreach ($vars['header'] as $k => $h) {
    if (!in_array($k, array('variant_name','srcfeature_name','fmin','fmax'))) {
      $header[] = str_replace(' ', '_', $h['data']);
    }
  }
  fputcsv($FILE, $header, "\t");
  // @debug print_r($header);

  // Match the template row to hapmap format.
  $template_row = array();
  $template_row['rs'] = 'NA';
  $template_row['alleles'] = 'NA';
  $template_row['chrom'] = 'NA';
  $template_row['pos'] = 'NA';
  $template_row['strand'] = 'NA';
  $template_row['assembly'] = 'NA';
  $template_row['center'] = 'NA';
  $template_row['prot'] = 'NA';
  $template_row['assay'] = 'NA';
  $template_row['panel'] = 'NA';
  $template_row['qccode'] = 'NA';
  foreach ($vars['template_row'] as $k => $v) {
    if (is_numeric($k)) {
      $template_row[$k] = 'NN';
    }
  }
  // @debug print_r($template_row);

  // Now, determine the query.
  // Unfortuntly we can't just use the same query as to generate the page since that is done
  // using two queries: 1 to select the variants for the page and 2 to grab the calls for
  // those variants/germplasm.
  // Instead we will use a single query to grab both variant and genotype call informtion
  // and we will optimize it by keeping each row to a single germplasm (less joins)
  // and programatically compiling it before printing.
  // With that in mind we first restrict the query based on the query parameters passed to us:
  $query_args = $vars['query_args'];
  $where = array();
  $args = array();
  $variant_where = array();
  $variant_args = array();

  // Sequence Ranges:
  // First get the arguements. There must always be a backbone and fmin even if the user
  // did not provide one.
  if (isset($query_args['start_backbone'])) {
    if (isset($query_args['start_pos'])) {
      $args[':sbackbone'] = $query_args['start_backbone'];
      $args[':sfmin'] = (int) $query_args['start_pos'];
    }
    else {
      $args[':sbackbone'] = $query_args['start_backbone'];
      $args[':sfmin'] = 1;
    }
    unset($query_args['start_backbone'], $query_args['start_pos']);
  }
  if (isset($query_args['end_backbone'])) {
    if (isset($query_args['end_pos'])) {
      $args[':ebackbone'] = $query_args['end_backbone'];
      $args[':efmin'] = (int) $query_args['end_pos'];
    }
    else {
      $args[':ebackbone'] = $query_args['end_backbone'];
      // @TODO: Switch to bigints then change this value to match the largest bigint.
      $args[':efmin'] = 2147483645;
    }
    unset($query_args['end_backbone'], $query_args['end_pos']);
  }
  // Now add the query where condition.
  if (isset($args[':sbackbone']) AND isset($args[':ebackbone'])) {
    $where[] = 'ROW(v.srcfeature_name, v.fmin) BETWEEN ROW(:sbackbone, :sfmin) AND ROW(:ebackbone,:efmin)';
  }
  elseif (isset($args[':sbackbone'])) {
    $where[] = 'ROW(v.srcfeature_name, v.fmin) >= ROW(:sbackbone, :sfmin)';
  }
  elseif (isset($args[':ebackbone'])) {
    $where[] = 'ROW(v.srcfeature_name, v.fmin) <= ROW(:ebackbone, :efmin)';
  }
  // Variant Type:
  if (isset($query_args['variant_type'])) {
    $where[] = 'variant_type=:variant_type';
    $args[':variant_type'] = $query_args['variant_type'];
    unset($query_args['variant_type']);
  }
  // Variant Name:
  $variant_name_filter = FALSE;
  if (isset($query_args['variant_name'])) {
    $where[] = "variant_name IN (VALUES('" . implode("'), ('", $query_args['variant_name']) . "'))";
    unset($query_args['variant_name']);
    $variant_name_filter = TRUE;
  }
  elseif (isset($query_args['variant_name_list'])) {
    $list_json = db_query('SELECT list FROM {ndg_matrix_variant_user_lists} WHERE list_id=:id',
      array(':id' => $query_args['variant_name_list']))->fetchField();
    $list = json_decode($list_json);
    $where[] = "variant_name IN (VALUES('" . implode("'), ('", $list) . "'))";
  }
  // So far all the filter criteria have applied to the variant list
  // so save that for use with counting.
  $variant_where = $where;
  $variant_args = $args;
  // Polymorphic:
  $polymorphic_filter = FALSE;
  $join = ' ';
  if (isset($query_args['polymorphic'])
    AND is_numeric($query_args['polymorphic']['first'])
    AND is_numeric($query_args['polymorphic']['second'])) {
        $join = "LEFT JOIN (
            SELECT a.variant_id, a.allele_call=b.allele_call as monomorphic
            FROM {mview_ndg_calls} a, {mview_ndg_calls} b
            WHERE a.variant_id=b.variant_id AND a.germplasm_id=:poly1 AND b.germplasm_id=:poly2
          ) p ON p.variant_id=v.variant_id ";
        $args[':poly1'] = $query_args['polymorphic']['first'];
        $args[':poly2'] = $query_args['polymorphic']['second'];
        $where[] = "p.monomorphic IS FALSE";
        $polymorphic_filter = TRUE;
  }
  // So far all the filter criteria have applied to the variant list
  // so save that for use with counting.
  $variant_where = $where;
  $variant_args = $args;
  // Germplasm:
  if (isset($query_args['germplasm_id']) AND !empty($query_args['germplasm_id'])) {
    $where[] = 'germplasm_id IN (:germplasm_ids)';
    $args[':germplasm_ids'] = $query_args['germplasm_id'];
    unset($query_args['germplasm_id']);
  }
  // Now determine the sort.
  if (isset($query_args['sort']) AND $query_args['sort'] == 'variant_name') {
    $order = array('variant_name', 'srcfeature_name', 'fmin');
  }
  else {
    $order = array('srcfeature_name', 'fmin');
  }
  // Finally we build the query.
  $query = '
    SELECT
      v.ndg_variants_id,
      v.variant_id,
      v.variant_name,
      v.srcfeature_name,
      v.fmin,
      v.fmax,
      c.germplasm_id,
      c.allele_call
    FROM {mview_ndg_variants} v
    LEFT JOIN {mview_ndg_calls} c USING(variant_id)
    ' . $join . '
    WHERE '. implode(' AND ', $where) .'
    ORDER BY '. implode(' ASC, ', $order) .' ASC';
  // Get an estimated number of rows.
  $count_query = 'SELECT v.variant_id FROM {mview_ndg_variants} v ' . $join;
  if (!empty($variant_where)) {
    $count_query .= ' WHERE '. implode(' AND ', $variant_where);
  }
  // @debug print "SQL: " . print_r($count_query, TRUE) . "; ARGS: " . print_r($variant_args, TRUE);
  $count_query = ndg_mview_query($vars['query_args']['partition'], $count_query, $variant_args, array('return_query' => TRUE));
  $total_rows = trpdownload_estimate_rows($count_query, $variant_args);
  print "Estimated number of rows: $total_rows.\n";
  // Now to start querying.
  // First start the transaction and create the cursor.
  print "\nStarting Export...\n";
  $cursor_name = 'download_calls';
  chado_query('BEGIN WORK');
  // @debug print_r($query); print_r($args);
  ndg_mview_query($vars['query_args']['partition'], 'DECLARE '.$cursor_name.' NO SCROLL CURSOR WITH HOLD FOR '.$query, $args);
  // Next start grabing chunks.
  $num_rows_exported = 0;
  while ($results = chado_query('FETCH FORWARD 1000 FROM '.$cursor_name)) {
    $some_results = FALSE;
    $last_percent = NULL;
    foreach ($results as $r) {
      $some_results = TRUE;
      // Our results are comming in one row per variant/germplasm combo.
      // As such we need to compile them into a row before printing to the file.
      // This is much faster then merging in the query!
      // Use the primary key of the variant mview to determine if this should be a new row.
      if (!isset($current_key) OR $current_key != $r->ndg_variants_id) {
        // Print the last row unless this is the first one.
        if (isset($row)) {

          // Set the allele column.
          $row['alleles'] = implode('/', $alleles);

          fputcsv($FILE, $row, "\t");
          $num_rows_exported++;
          // Update progress if we've reached an increment.
          $percent = round(($num_rows_exported/$total_rows)*100);
          if ( (($percent % 5) == 0) AND ($percent != 100) AND ($percent != $last_percent)) {
            print $percent . '%...';
            db_query('UPDATE {tripal_jobs} SET progress=:percent WHERE job_id=:job_id',
              array(':percent' => $percent, ':job_id' => $job_id));
              chado_query('COMMIT WORK');
          }
          $last_percent = $percent;
        }

        // Reset alleles.
        $alleles = array();

        // Start a new row.
        $current_key = $r->ndg_variants_id;
        $row = $template_row;
        $row['rs'] = $r->variant_name;
        $row['chrom'] = $r->srcfeature_name;
        $row['pos'] = $r->fmin;

      }
      // Add the call to the current row.
      $row[ $r->germplasm_id ] = $r->allele_call;
      foreach (str_split($r->allele_call) as $call) {
        $alleles[$call] = $call;
      }

      // @debug print_r($row);
    }
    if (!$some_results) { break; }
  }
  // print the last row.
  if (!empty($row)) {
    $row['alleles'] = implode('/', $alleles);
    fputcsv($FILE, $row, "\t");
    $num_rows_exported++;
  }
  // Finally, complete.
  chado_query('CLOSE '.$cursor_name);
  chado_query('COMMIT WORK');
  chado_query('END');
  fclose($FILE);
  db_query('UPDATE {tripal_jobs} SET progress=:percent WHERE job_id=:job_id',
    array(':percent' => 100, ':job_id' => $job_id));
    print "100%\n";
  print "\nExport Complete ($num_rows_exported rows).\n\n";
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

/**
 * Determine the file suffix for the file to be generated.
 */
function trpdownload_genotype_matrix_download_get_txt_suffix($vars) {
  return 'txt';
}
