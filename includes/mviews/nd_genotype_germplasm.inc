<?php

/**
 * Register a Materialized View to keep track of the germplasm genotyped.
 *
 */
function nd_genotypes_register_nd_genotype_germplasm_mview() {
  global $user;

  $name = "nd_genotype_germplasm";
  $comment = 'A list of the germplasm genotyped (in the nd_genotype_experiment table).';

  $schema = array (
    'description' => 'A list of the germplasm genotyped (in the nd_genotype_experiment table).',
    'table' => 'nd_genotype_germplasm',
    'fields' => array (
      'stock_id' => array (
        'type' => 'int',
        'not null' => true,
      ),
      'name' => array (
        'type' => 'varchar',
        'length' => '255',
        'not null' => true,
      ),
      'uniquename' => array (
        'type' => 'varchar',
        'length' => '255',
        'not null' => true,
      ),
      'organism_id' => array (
        'type' => 'int',
        'not null' => true,
      ),
      'genus' => array (
        'type' => 'varchar',
        'length' => '255',
        'not null' => true,
      ),
      'species' => array (
        'type' => 'varchar',
        'length' => '255',
        'not null' => true,
      ),
      'num_genotypes' => array (
        'type' => 'int',
        'not null' => true,
      ),
    ),
    'indexes' => array (
      'organism_id_idx' => array (
        0 => 'organism_id',
      ),
      'stock_id_idx' => array (
        0 => 'stock_id',
      ),
    ),
  );

  $query = "SELECT
      s.stock_id,
      s.name,
      s.uniquename,
      o.organism_id,
      o.genus,
      o.species,
      count(nd.*) as num_genotypes
    FROM nd_genotype_experiment nd
    LEFT JOIN stock s ON s.stock_id=nd.stock_id
    LEFT JOIN organism o ON o.organism_id=s.organism_id
    GROUP BY s.stock_id, s.name, s.uniquename, o.organism_id, o.genus, o.species
    ORDER BY o.genus, o.species, s.name";

  // First check to see if it's already registered/created
  $mview_id = tripal_mviews_get_mview_id($name);
  if (!$mview_id) {

    // If it doesn't already exist then create it
    tripal_add_mview($name, 'nd_genotypes', $schema, $query, $comment);
    $mview_id = tripal_mviews_get_mview_id($name);
  }

  // Then Populate it.
  tripal_add_job(
    "Populate the nd_genotype_germplasm materialized view.",
    'nd_genotypes',
    'tripal_populate_mview',
    array($mview_id),
    $user->uid
  );

}