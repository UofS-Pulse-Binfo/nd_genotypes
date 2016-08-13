<?php
/**
 * @file
 * Provides functionality for sync'ing data to the materialized views.
 */

/**
 * Method 1: Nd Experiment Tables.
 */
function nd_genotypes_update_mview_for_nd_exp($job_id) {
  drupal_set_message('This storage method is not currently supported.', 'error');
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
  nd_genotypes_create_mview_ndg_variants($ndg_variants);
  nd_genotypes_create_mview_ndg_calls($ndg_call);

  // And clear them if they do already exist.
  chado_query("TRUNCATE {".$ndg_variants."}");
  chado_query("TRUNCATE {".$ndg_call."}");

  // First, populate the mview_ndg_calls table.
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
      gc.marker_data
    FROM {genotype_call} gc
      LEFT JOIN {feature} m ON m.feature_id=gc.marker_id
      LEFT JOIN {featureprop} mt ON mt.feature_id=m.feature_id AND mt.type_id IN (SELECT cvterm_id FROM cvterm WHERE name='marker_type')
      LEFT JOIN {stock} s ON s.stock_id=gc.stock_id
      LEFT JOIN {stock}_relationship sr ON sr.subject_id=s.stock_id AND sr.type_id IN (SELECT cvterm_id FROM cvterm WHERE name='is_extracted_from')
      LEFT JOIN {stock} g ON g.stock_id=sr.object_id
      LEFT JOIN {genotype} a ON a.genotype_id=gc.genotype_id
    ORDER BY gc.variant_id ASC, gc.marker_id ASC, g.stock_id ASC
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
      allele_call,
      marker_data)
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
 * Method 3: Genotype Stock Table.
 */
function nd_genotypes_update_mview_for_stock_genotype($job_id) {
  drupal_set_message('This storage method is not currently supported.', 'error');
}
