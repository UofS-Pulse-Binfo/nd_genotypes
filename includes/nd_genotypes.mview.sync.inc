<?php
/**
 * @file
 * Provides functionality for sync'ing data to the materialized views.
 */

/**
 * Method 1: Nd Experiment Tables.
 */
function nd_genotypes_update_mview_for_nd_exp($job_id) {
  $ndg_variants = 'mview_ndg_variants';
  $ndg_call = 'mview_ndg_calls';

  // Create both tables if they don't already exists.
  nd_genotypes_create_mview_ndg_variants($ndg_variants);
  nd_genotypes_create_mview_ndg_calls($ndg_call);

  // And clear them if they do already exist.
  chado_query("TRUNCATE {".$ndg_variants."}");
  chado_query("TRUNCATE {".$ndg_call."}");

  // First, populate the mview_ndg_calls table.
  // This is basically a materialized version of the genotype_call table itself :-).
  $query = "
    SELECT
      v.feature_id as variant_id,
      m.feature_id as marker_id,
      m.name as marker_name,
      mt.value as marker_type,
      s.stock_id as stock_id,
      s.name as stock_name,
      germ.stock_id as germplasm_id,
      germ.name as germplasm_name,
      ndp.project_id as project_id,
      g.genotype_id as genotype_id,
      g.description as allele_call
    FROM {nd_experiment_genotype} ndg
      LEFT JOIN {nd_experiment} exp ON ndg.nd_experiment_id=exp.nd_experiment_id
      LEFT JOIN {genotype} g ON g.genotype_id=ndg.genotype_id
      LEFT JOIN {feature_genotype} fg ON fg.genotype_id=g.genotype_id
      LEFT JOIN {feature} m ON m.feature_id=fg.feature_id
      LEFT JOIN {featureprop} mt ON mt.feature_id=m.feature_id
        AND mt.type_id IN (SELECT cvterm_id FROM cvterm WHERE name='marker_type')
      LEFT JOIN {feature_relationship} fr ON fr.subject_id=m.feature_id
        AND fr.type_id IN (SELECT cvterm_id FROM cvterm WHERE name='is_marker_of')
      LEFT JOIN {feature} v ON v.feature_id=fr.object_id
      LEFT JOIN {nd_experiment_stock} nds ON nds.nd_experiment_id=ndg.nd_experiment_id
      LEFT JOIN {stock} s ON s.stock_id=nds.stock_id
      LEFT JOIN {stock_relationship} sr ON sr.subject_id=s.stock_id
        AND sr.type_id IN (SELECT cvterm_id FROM cvterm WHERE name='is_extracted_from')
      LEFT JOIN {stock germ} ON germ.stock_id=sr.object_id
      LEFT JOIN {nd_experiment_project} ndp ON ndp.nd_experiment_id=ndg.nd_experiment_id
      WHERE exp.nd_experiment_id IS NOT NULL AND ndp.nd_experiment_id IS NOT NULL
        AND germ.stock_id IS NOT NULL AND s.stock_id IS NOT NULL
      ORDER BY v.feature_id ASC, m.feature_id ASC, s.stock_id ASC
    LIMIT 20000
    ";
  chado_query("
    INSERT INTO {".$ndg_call."} (
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
      allele_call)
    " . $query);

  // Now, calculate the variant list.
  // @TODO: Restrict locations to a given assembly.
  $query = "
    SELECT
      call.variant_id,
      v.name as variant_name,
      loc.srcfeature_id as srcfeature_id,
      b.name as srcfeature_name,
      loc.fmin,
      loc.fmax
    FROM {mview_ndg_calls} call
      LEFT JOIN {feature} v ON v.feature_id=call.variant_id
      LEFT JOIN {featureloc} loc ON loc.feature_id=call.variant_id
      LEFT JOIN {feature} b ON b.feature_id=loc.srcfeature_id
    GROUP BY call.variant_id, v.name, loc.srcfeature_id, b.name, loc.fmin, loc.fmax
  ";
  chado_query("
    INSERT INTO {".$ndg_variants."} (
      variant_id,
      variant_name,
      srcfeature_id,
      srcfeature_name,
      fmin,
      fmax)
    " . $query);

  // Update the cached lists.
  nd_genotypes_update_mview_cache();
}

/**
 * Method 2: Genotype Call Custom Table.
 *
 * Genotype_call custom table:
 *   - genotype_call_id: primary key for this table,
 *   - variant_id: the feature_id of the variant,
 *   - marker_id: the feature_id of the marker,
 *   - genotype_id: the genotype_id of the allele call for the given marker/stock combination
 *   - project_id: the project_id of the over-aching project
 *   - stock_id: the stock_id of the germplasm assayed
 *   - meta_data: JSONB blob to store meta-data
 *
 * Approach:
 *  1) using a single query to populate the entire table
 *  2) iterate over that table
 *      a) refining the consensus call and
 *      b) filling in the marker details.
 * Notice that we will attempt to create the table if it doesn't already exist.
 */
function nd_genotypes_update_mview_for_genotype_call($job_id) {
  $ndg_variants = 'mview_ndg_variants';
  $ndg_call = 'mview_ndg_calls';

  // Create both tables if they don't already exists.
  print "\nCreating $ndg_variants if it doesn't already exist...\n";
  nd_genotypes_create_mview_ndg_variants($ndg_variants);
  print "Creating $ndg_call if it doesn't already exist...\n";
  nd_genotypes_create_mview_ndg_calls($ndg_call);

  // And clear them if they do already exist.
  print "\nTruncating $ndg_variants...\n";
  chado_query("TRUNCATE {".$ndg_variants."}");
  print "Truncating $ndg_call...\n";
  chado_query("TRUNCATE {".$ndg_call."}");

  //-------------------------
  // mview_ndg_calls
  print "\nSync'ing genotypes to the mview_ndg_calls mview...\n";

  // Drop all the indexes on the mview to make copying faster.
  print "\tDropping indexes on mview_ndg_calls...\n";
  nd_genotypes_drop_indexes('mview_ndg_calls');
  print "\t\tComplete.\n";

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
  print "\tDetermined there should be " . sizeof($genotypes) . " chunks.\n";

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
      LEFT JOIN {featureprop} mt ON mt.feature_id=m.feature_id AND mt.type_id IN (SELECT cvterm_id FROM cvterm WHERE name='marker_type')
      LEFT JOIN {stock} s ON s.stock_id=gc.stock_id
      LEFT JOIN {stock}_relationship sr ON sr.subject_id=s.stock_id AND sr.type_id IN (SELECT cvterm_id FROM cvterm WHERE name='is_extracted_from')
      LEFT JOIN {stock} g ON g.stock_id=sr.object_id
      LEFT JOIN {genotype} a ON a.genotype_id=gc.genotype_id
    WHERE gc.genotype_id=:id
    ";

  $i = 0;
  $total = sizeof($genotypes);
  foreach ($genotypes as $genotype_id) {
    $i++;

    print "\tWorking on genotype_id=$genotype_id ($i of $total)...\n";

    // Copy it to a file.
    print "\t\tCopying to file...\n";
    $result = chado_query("COPY (" . $query . ") TO '/tmp/mview_ndg_calls.copy'", array(':id' => $genotype_id))->rowCount();

    // Then copy it back into the mview.
    print "\t\tCopying it into the mview...\n";
    $result = chado_query("COPY {".$ndg_call."} (
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
          meta_data) FROM '/tmp/mview_ndg_calls.copy'")->rowCount();
    print "\t\tFinished $result records.\n";
  }

  // Create the indexes you dropped earlier.
  print "\tCreating Indexes\n";
  nd_genotypes_create_mview_ndg_calls_indexes($ndg_call);

  //-------------------------
  // mview_ndg_variants
  print "\nSync'ing genotypes to the mview_ndg_variants mview...\n";

  // Drop all the indexes on the mview to make copying faster.
  print "\tDropping indexes on mview_ndg_variants...\n";
  nd_genotypes_drop_indexes('mview_ndg_variants');
  print "\t\tComplete.\n";

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
  $total_min = chado_query('SELECT min(variant_id) FROM {mview_ndg_calls}')->fetchField();
  $total_max = chado_query('SELECT max(variant_id) FROM {mview_ndg_calls}')->fetchField();
  // Seems relatively performant at 50 million.
  $chunk_size = 500000;
  $num_chunks = 1;
  $total = $total_max - $total_min;
  if ($chunk_size <= $total) {
   $num_chunks = ($total_max-$total_min) / $chunk_size;
  }

  print "\tDetermined there should be " . ceil($num_chunks) . " chunks.\n";

  // Now, populate the mview_ndg_variants table for each genotype/allele
  // based on the mview_ndg_calls table :-).
  $query = "
    SELECT
      call.variant_id,
      v.name as variant_name,
      loc.srcfeature_id as srcfeature_id,
      b.name as srcfeature_name,
      loc.fmin,
      loc.fmax
    FROM {mview_ndg_calls} call
      LEFT JOIN {feature} v ON v.feature_id=call.variant_id
      LEFT JOIN {featureloc} loc ON loc.feature_id=call.variant_id
      LEFT JOIN {feature} b ON b.feature_id=loc.srcfeature_id
    WHERE call.variant_id BETWEEN :min AND :max
    ";

  $total = $max - $min;
  $min = $total_min;
  for ($i=1; $i <= $num_chunks; $i++) {
    $max = $min + $chunk_size - 1;

    print "\tWorking on $min to $max ($i of ".ceil($num_chunks).")...\n";

    // Copy it to a file.
    print "\t\tCopying to file...\n";
    $result = chado_query("COPY (" . $query . ") TO '/tmp/mview_ndg_variants.copy'", array(':min' => $min, ':max' => $max))->rowCount();

    // Then we have to collapse it.
    print "\t\tCollapsing it to unique variants...\n";
    system('sort --unique --output /tmp/mview_ndg_variants.copy.unique /tmp/mview_ndg_variants.copy');

    // Then copy it back into the mview.
    print "\t\tCopying it into the mview...\n";
    $result = chado_query("COPY {".$ndg_variants."} (
      variant_id,
      variant_name,
      srcfeature_id,
      srcfeature_name,
      fmin,
      fmax) FROM '/tmp/mview_ndg_variants.copy.unique'")->rowCount();
    print "\t\tFinished $result records.\n";

    $min += $chunk_size;
  }

  // Create the indexes you dropped earlier.
  print "\tCreating Indexes\n";
  nd_genotypes_create_mview_ndg_variants_indexes();

  // Update the cached lists.
  print "\nUpdating cached lists used for select boxes and what-not\n";
  nd_genotypes_update_mview_cache();
}

/**
 * Method 3: Genotype Stock Table.
 */
function nd_genotypes_update_mview_for_stock_genotype($job_id) {
  drupal_set_message('This storage method is not currently supported.', 'error');
}
