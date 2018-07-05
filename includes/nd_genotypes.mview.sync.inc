<?php
/**
 * @file
 * Provides functionality for sync'ing data to the materialized views.
 */

function nd_genotypes_update_mview($partitions, $job_id) {

  print "Updating the materialized views for the following partitions: " . implode(', ', $partitions) . ".\n\n";

  $num_processes = (sizeof($partitions) * 4) + 1;
  $percent_increment = floor(100 / $num_processes);

  // For each partition...
  $current_percent = 0;
  foreach ($partitions as $partition) {
    print "Partition: $partition...\n";

    $partition = strtolower($partition);

    print "   Sync'ing genotypes to the mview_ndg_calls mview...\n";
    nd_genotypes_update_mview_ndg_calls($partition, $job_id);
    $current_percent += $percent_increment;
    db_query('UPDATE {tripal_jobs} SET progress=:percent WHERE job_id=:id',
      array(':percent' => $current_percent, ':id' => $job_id));

    print "   Sync'ing genotypes to the mview_ndg_variants mview...\n";
    nd_genotypes_update_mview_ndg_variants($partition, $job_id);
    $current_percent += $percent_increment;
    db_query('UPDATE {tripal_jobs} SET progress=:percent WHERE job_id=:id',
      array(':percent' => $current_percent, ':id' => $job_id));

    print "   Sync'ing genotypes to the mview_ndg_germplasm_genotyped mview...\n";
    nd_genotypes_update_mview_ndg_germplasm_genotyped($partition, $job_id);
    $current_percent += $percent_increment;
    db_query('UPDATE {tripal_jobs} SET progress=:percent WHERE job_id=:id',
      array(':percent' => $current_percent, ':id' => $job_id));

    // Update the cached lists.
    print "   Updating cached lists used for select boxes and what-not\n";
    nd_genotypes_update_mview_cache($partition, TRUE);
    $current_percent += $percent_increment;
    db_query('UPDATE {tripal_jobs} SET progress=:percent WHERE job_id=:id',
      array(':percent' => $current_percent, ':id' => $job_id));

  }

  // Summarize.
  nd_genotypes_summarize_mviews();
  $current_percent += $percent_increment;
  db_query('UPDATE {tripal_jobs} SET progress=:percent WHERE job_id=:id',
    array(':percent' => $current_percent, ':id' => $job_id));

  // Ensure complete.
  db_query('UPDATE {tripal_jobs} SET progress=:percent WHERE job_id=:id',
    array(':percent' => 100, ':id' => $job_id));
}

/**
 * Update mview_ndg_calls Materialized view.
 *
 * @param $partition
 *   The partition to update the materialized view of.
 * @param $job_id
 *   The id of the tripal job whose status should be updated.
 *
 * Notice that we will attempt to create the table if it doesn't already exist.
 */
function nd_genotypes_update_mview_ndg_calls($partition, $job_id) {

  // Determine the name of the mview table in this partition.
  $table_name = 'mview_ndg_'.$partition.'_calls';

  // Create the table if it doesn't already exist.
  print "    - Creating $table_name if it doesn't already exist...\n";
  nd_genotypes_create_mview_ndg_calls($table_name);

  // And Clear it if it does exist.
  print "    - Truncating $table_name...\n";
  chado_query("TRUNCATE {".$table_name."}");

  // Drop all the indexes on the mview to make copying faster.
  print "    - Dropping indexes on $table_name...\n";
  nd_genotypes_drop_indexes($table_name);
  print "       - Complete.\n";

  // We would like to chunk the dataset into hopefully reasonable pieces for updating
  // the materialized view. However, we would also like to avoid sorting first since
  // the has a large performance hit. Our soluition is to break the dataset by the
  // allele/genotype for each call. While this makes no sense from a biological standpoint
  // it is at least something we can assume is present (since calls are genotypes;-) ).
  // This approach will suffer if there are a large number of alleles (ie: MNPs or indels)
  // especially if the overall dataset is small. That said it is well suited to SNP data
  // which we forsee being the bulk of the data.
  // For this approach we first need to get the list of genotype_ids to use for chunk
  // delimiters.
  $genotypes = chado_query('SELECT genotype_id FROM {genotype}')->fetchCol();
  print "    - Determined there should be " . sizeof($genotypes) . " chunks.\n";

  // Retrieve the marker type information.
  $markerprop_type_id = nd_genotypes_get_type_id('Marker Type');
  if (!$markerprop_type_id) {
    drush_report_error('Marker Type not set. Please go to Admin > Tripal > Extensions > ND Genotypes > Settings to configure this.');
  }

  // Retrieve the stock relationship information.
  $stock_rel = nd_genotypes_get_type_id('Stock Relationship');
  if ($stock_rel['position'] == 'subject') {
    $sample_pos = 'subject_id';
    $germ_pos = 'object_id';
  }
  else {
    $sample_pos = 'object_id';
    $germ_pos = 'subject_id';
  }
  if (!$stock_rel['type_id']) {
    drush_report_error('Sample => Germplasm Relationship Type not set. Please go to Admin > Tripal > Extensions > ND Genotypes > Settings to configure this.');
  }

  // Now, populate the mview_ndg_calls table for each genotype/allele.
  // This is basically a materialized version of the genotype_call table itself :-).
  $query = "
    SELECT
      gc.variant_id,
      gc.marker_id,
      m.name as marker_name,
      mt.value as marker_type,
      gc.stock_id,
      s.name as stock_name,
      g.stock_id as germplasm_id,
      g.name as germplasm_name,
      gc.project_id,
      gc.genotype_id,
      a.description as allele_call,
      gc.meta_data
    FROM {genotype_call} gc
      LEFT JOIN {feature} m ON m.feature_id=gc.marker_id
      LEFT JOIN {featureprop} mt ON mt.feature_id=m.feature_id AND mt.type_id=:markerprop_type
      LEFT JOIN {stock} s ON s.stock_id=gc.stock_id
      LEFT JOIN {organism} o ON o.organism_id=s.organism_id
      LEFT JOIN {stock}_relationship sr ON sr.".$sample_pos."=s.stock_id AND sr.type_id=:stockrel_type
      LEFT JOIN {stock} g ON g.stock_id=sr.".$germ_pos."
      LEFT JOIN {genotype} a ON a.genotype_id=gc.genotype_id
    WHERE gc.genotype_id=:id AND LOWER(o.genus)=LOWER(:partition)
    ";
  // @PARTITION We assume partitions are by genus here...

  $i = 0;
  $total = sizeof($genotypes);
  foreach ($genotypes as $genotype_id) {
    $i++;

    print "    - Working on genotype_id=$genotype_id ($i of $total)...\n";

    // Copy it to a file.
    print "       - Copying to file...\n";
    $result = ndg_remote_chado_copy("COPY (" . $query . ") TO '/tmp/mview_ndg_calls.copy'",
      array(':id' => $genotype_id, ':partition' => $partition,
        ':markerprop_type' => $markerprop_type_id, ':stockrel_type' => $stock_rel['type_id']));


    // Then copy it back into the mview.
    print "       - Copying it into the mview...\n";
    $result = ndg_remote_chado_copy("COPY {".$table_name."} (
          variant_id,
          marker_id,
          marker_name,
          marker_type,
          stock_id,
          stock_name,
          germplasm_id,
          germplasm_name,
          project_id,
          genotype_id,
          allele_call,
          meta_data) FROM '/tmp/mview_ndg_calls.copy'");
    print "       - Finished this chunk.\n";
  }

  // Create the indexes you dropped earlier.
  print "    - Creating Indexes\n";
  nd_genotypes_create_mview_ndg_calls_indexes($table_name);

}

/**
 * Update mview_ndg_calls Materialized view.
 *
 * @param $partition
 *   The partition to update the materialized view of.
 * @param $job_id
 *   The id of the tripal job whose status should be updated.
 *
 * Notice that we will attempt to create the table if it doesn't already exist.
 */
function nd_genotypes_update_mview_ndg_variants($partition, $job_id) {
  $pg_version = nd_genotypes_get_postgresql_version();

  // Determine the name of the mview table in this partition.
  $table_name = 'mview_ndg_'.$partition.'_variants';
  $ndg_calls_name = 'mview_ndg_'.$partition.'_calls';

  // Create both tables if they don't already exists.
  print "    - Creating $table_name if it doesn't already exist...\n";
  nd_genotypes_create_mview_ndg_variants($table_name);

  // And clear them if they do already exist.
  print "    - Truncating $table_name...\n";
  chado_query("TRUNCATE {".$table_name."}");

  // Drop all the indexes on the mview to make copying faster.
  print "    - Dropping indexes on $table_name...\n";
  nd_genotypes_drop_indexes($table_name);
  print "       - Complete.\n";

  // We would like to chunk the dataset into hopefully reasonable pieces for updating
  // the materialized view. However, we would also like to avoid sorting first since
  // the has a large performance hit. Our soluition is to break the dataset by the
  // allele/genotype for each call. While this makes no sense from a biological standpoint
  // it is at least something we can assume is present (since calls are genotypes;-) ).
  // This approach will suffer if there are a large number of alleles (ie: MNPs or indels)
  // especially if the overall dataset is small. That said it is well suited to SNP data
  // which we forsee being the bulk of the data.
  // For this approach we first need to get the list of genotype_ids to use for chunk
  // delimiters.
  $total_min = chado_query("SELECT min(variant_id) FROM {".$ndg_calls_name."}")->fetchField();
  $total_max = chado_query("SELECT max(variant_id) FROM {".$ndg_calls_name."}")->fetchField();
  // Seems relatively performant at 50 million.
  $chunk_size = 500000;
  $num_chunks = 1;
  $total = $total_max - $total_min;
  if ($chunk_size <= $total) {
   $num_chunks = ($total_max-$total_min) / $chunk_size;
  }

  print "    - Determined there should be " . ceil($num_chunks) . " chunks.\n";

  // Now, populate the mview_ndg_variants table for each genotype/allele
  // based on the mview_ndg_calls table :-).
  // Because this only pulls from the calls which are already filtered to the given partition
  // we don't need to use the partition here.
  $variantprop_type_id = nd_genotypes_get_type_id('Variant Type');
  if ($pg_version == 9.3 ) {
  $query = "
      SELECT
        call.variant_id,
        v.name as variant_name,
        vt.value as variant_type,
        loc.srcfeature_id as srcfeature_id,
        b.name as srcfeature_name,
        loc.fmin,
        loc.fmax,
        row_to_json((SELECT d FROM (SELECT loc.strand as strand, loc.featureloc_id as featureloc_id) d)) as meta_data
      FROM {".$ndg_calls_name."} call
        LEFT JOIN {feature} v ON v.feature_id=call.variant_id
        LEFT JOIN {featureprop} vt ON vt.feature_id=v.feature_id AND vt.type_id=:variantprop_type
        LEFT JOIN {featureloc} loc ON loc.feature_id=call.variant_id
        LEFT JOIN {feature} b ON b.feature_id=loc.srcfeature_id
      WHERE call.variant_id BETWEEN :min AND :max
      ";
  }
  elseif ($pg_version >= 9.4) {
    $query = "
      SELECT
        call.variant_id,
        v.name as variant_name,
        vt.value as variant_type,
        loc.srcfeature_id as srcfeature_id,
        b.name as srcfeature_name,
        loc.fmin,
        loc.fmax,
        json_build_object('strand', loc.strand, 'featureloc_id', loc.featureloc_id) as meta_data
      FROM {".$ndg_calls_name."} call
        LEFT JOIN {feature} v ON v.feature_id=call.variant_id
        LEFT JOIN {featureprop} vt ON vt.feature_id=v.feature_id AND vt.type_id=:variantprop_type
        LEFT JOIN {featureloc} loc ON loc.feature_id=call.variant_id
        LEFT JOIN {feature} b ON b.feature_id=loc.srcfeature_id
      WHERE call.variant_id BETWEEN :min AND :max
      ";
  }
  else {
    tripal_report_error(
      'nd_genotypes',
      TRIPAL_ERROR,
      'Unable to populate materialized view due to incompatible postgresql version. You are using :curr and this module requires at least 9.3.',
      array( ':curr' => $pg_version )
    );
  }

  $total = $max - $min;
  $min = $total_min;
  for ($i=1; $i <= $num_chunks; $i++) {
    $max = $min + $chunk_size - 1;

    print "    - Working on $min to $max ($i of ".ceil($num_chunks).")...\n";

    // Copy it to a file.
    print "       - Copying to file...\n";
    $result = ndg_remote_chado_copy(
        "COPY (" . $query . ") TO '/tmp/mview_ndg_variants.copy'",
        array(':min' => $min, ':max' => $max, ':variantprop_type' => $variantprop_type_id));

    if (!file_exists('/tmp/mview_ndg_variants.copy')) {
      drush_log('No variants selected by: '
        . str_replace(array(':min', ':max'), array($min, $max), str_replace(array('{','}'), array('chado.',''), $query)),
        'warning');
      continue;
    }

    // Then we have to collapse it.
    print "       - Collapsing it to unique variants...\n";
    system('sort --unique --output /tmp/mview_ndg_variants.copy.unique /tmp/mview_ndg_variants.copy');

    // Then copy it back into the mview.
    print "       - Copying it into the mview...\n";
    $result = ndg_remote_chado_copy("COPY {".$table_name."} (
      variant_id,
      variant_name,
      variant_type,
      srcfeature_id,
      srcfeature_name,
      fmin,
      fmax,
      meta_data)
      FROM '/tmp/mview_ndg_variants.copy.unique'");
    print "       - Finished $result records.\n";

    $min += $chunk_size;
  }

  // Create the indexes you dropped earlier.
  print "    - Creating Indexes\n";
  nd_genotypes_create_mview_ndg_variants_indexes($table_name);
}

/**
 * Update mview_ndg_germplasm_genotyped Materialized view.
 *
 * @param $partition
 *   The partition to update the materialized view of.
 * @param $job_id
 *   The id of the tripal job whose status should be updated.
 *
 * Notice that we will attempt to create the table if it doesn't already exist.
 */
function nd_genotypes_update_mview_ndg_germplasm_genotyped($partition, $job_id) {

  $table_name = 'mview_ndg_germplasm_genotyped';
  $ndg_calls_name = 'mview_ndg_'.$partition.'_calls';

  // Create both tables if they don't already exists.
  print "    - Creating $table_name if it doesn't already exist...\n";
  nd_genotypes_create_mview_ndg_germplasm_genotyped($table_name);

  // And clear them if they do already exist.
  print "    - Removing records related to this partition ($partition)...\n";
  chado_query("DELETE FROM {".$table_name."} WHERE partition = :partition",
    array(':partition' => $partition));

  // Again, since this pulls from the calls which are already filtered to partition,
  // we don't have to worry about partition again here.
  $query = "WITH RECURSIVE t AS (
      (SELECT germplasm_id
        FROM {mview_ndg_calls} m
        ORDER BY germplasm_id
        LIMIT 1)
      UNION ALL
      SELECT
          (SELECT germplasm_id
            FROM {mview_ndg_calls} m
            WHERE germplasm_id > t.germplasm_id
            ORDER BY germplasm_id
            LIMIT 1)
        FROM t
        WHERE t.germplasm_id IS NOT NULL
    )
    SELECT t.germplasm_id, s.name as germplasm_name, s.organism_id, o.genus, :partition
      FROM t
      LEFT JOIN {stock} s ON s.stock_id=t.germplasm_id
      LEFT JOIN {organism} o ON o.organism_id=s.organism_id
      WHERE t.germplasm_id IS NOT NULL";
    $insert_query = 'INSERT INTO {' . $table_name . '} (germplasm_id, germplasm_name, organism_id, organism_genus, partition) ' . $query;
    ndg_mview_query($partition, $insert_query, array(':partition' => $partition));

}

/**
 * Update cached information after the materialized view has been updated.
 */
function nd_genotypes_update_mview_cache($partition, $print = FALSE) {

  $ndg_variants = 'mview_ndg_'.$partition.'_variants';
  $ndg_calls = 'mview_ndg_'.$partition.'_calls';

  // The types of variants (e.g. SNP, MNP, Indel) with the key being the type_id.
  // @todo this should be based off the mview_ndg_variants which will also
  // ensure it is filtered to partition.
  if ($print) print "    - Determining the types of variants...\n";
  $resource = ndg_mview_query($partition, "
    WITH RECURSIVE t AS (
      (SELECT variant_type
        FROM {mview_ndg_variants}
        ORDER BY variant_type
        LIMIT 1)
      UNION ALL
      SELECT
          (SELECT variant_type
            FROM {mview_ndg_variants}
            WHERE variant_type > t.variant_type
            ORDER BY variant_type
            LIMIT 1)
        FROM t
        WHERE t.variant_type IS NOT NULL
    )
    SELECT t.variant_type FROM t WHERE t.variant_type IS NOT NULL");  $types = array();
  foreach ($resource as $r) {
    $types[$r->variant_type] = $r->variant_type;
  }
  variable_set('nd_genotypes_'.$partition.'_variant_types', serialize($types));

  // The types of markers (e.g. GBS, Exome Capture) as stored in the featureprop table.
  // Already filtered to partition since it pulls from calls.
  if ($print) print "    - Determining the types of markers...\n";
  $resource = ndg_mview_query($partition, "
    WITH RECURSIVE t AS (
      (SELECT marker_type
        FROM {mview_ndg_calls}
        ORDER BY marker_type
        LIMIT 1)
      UNION ALL
      SELECT
          (SELECT marker_type
            FROM {mview_ndg_calls}
            WHERE marker_type > t.marker_type
            ORDER BY marker_type
            LIMIT 1)
        FROM t
        WHERE t.marker_type IS NOT NULL
    )
    SELECT t.marker_type FROM t WHERE t.marker_type IS NOT NULL");
  $types = array();
  foreach ($resource as $r) {
    $types[$r->marker_type] = $r->marker_type;
  }
  variable_set('nd_genotypes_'.$partition.'_marker_types', serialize($types));

}

/**
 * Summarize the materialized views.
 *
 * This information is used on the admin pages.
 */
function nd_genotypes_summarize_mviews($print = FALSE) {

  if ($print) print "    - Creating the table if it doesn't already exist...\n";
  nd_genotypes_create_mview_summary();

  // And clear them if they do already exist.
  if ($print) print "    - Truncating mview_ndg_summary...\n";
  chado_query("TRUNCATE {mview_ndg_summary}");

  if ($print) print "    - Determining the partitions sync'd...\n";
  $resource = ndg_mview_query($partition, "
    WITH RECURSIVE t AS (
      (SELECT partition
        FROM {mview_ndg_germplasm_genotyped}
        ORDER BY partition
        LIMIT 1)
      UNION ALL
      SELECT
          (SELECT partition
            FROM {mview_ndg_germplasm_genotyped}
            WHERE partition > t.partition
            ORDER BY partition
            LIMIT 1)
        FROM t
        WHERE t.partition IS NOT NULL
    )
    SELECT t.partition FROM t WHERE t.partition IS NOT NULL");
  $partitions = array();
  foreach ($resource as $r) {
    $partitions[$r->partition] = $r->partition;
  }
  variable_set('nd_genotypes_partitions', serialize($partitions));

  foreach ($partitions as $partition) {

    if ($print) print "    - Summarizing $partition...\n";

    // Number of Genotype Calls.
    chado_query('
      INSERT INTO {mview_ndg_summary} (stat_id, stat_name, stat_unit, partition, value)
      SELECT :stat_id, :stat_name, :stat_unit, :partition, count(*) as value
        FROM {mview_ndg_'.$partition.'_calls}',
      array(
        ':stat_id' => 'num_calls',
        ':stat_name' => 'Number of Genotype Calls',
        ':stat_unit' => 'count',
        ':partition' => $partition
      ));

    // Number of Variants.
    chado_query('
      INSERT INTO {mview_ndg_summary} (stat_id, stat_name, stat_unit, partition, value)
      SELECT :stat_id, :stat_name, :stat_unit, :partition, count(*) as value
        FROM (SELECT variant_id FROM {mview_ndg_'.$partition.'_variants} GROUP BY variant_id) as variants',
      array(
        ':stat_id' => 'num_variants',
        ':stat_name' => 'Number of Variants',
        ':stat_unit' => 'count',
        ':partition' => $partition
      ));

    // Number of Germplasm Genotyped.
    chado_query('
      INSERT INTO {mview_ndg_summary} (stat_id, stat_name, stat_unit, partition, value)
      SELECT :stat_id, :stat_name, :stat_unit, :partition, count(*) as value
        FROM {mview_ndg_germplasm_genotyped}
        WHERE partition=:partition',
      array(
        ':stat_id' => 'num_germplasm',
        ':stat_name' => 'Number of Germplasm',
        ':stat_unit' => 'count',
        ':partition' => $partition
      ));
  }
}
