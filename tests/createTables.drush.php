<?php
/*********** RUN WITH DRUSH **************************************************
 * This script creates the tables needed for running tests. It needs to be
 * done outside of tests due to problems linked to use of database transactions
 * in tests.
 *
 * drush php-script tests/createTables.drush.php 
 */
$partition = 'tripalus';
$calls_table = 'mview_ndg_'.$partition.'_calls';
nd_genotypes_create_mview_ndg_calls($calls_table);
print "Created $calls_table.\n";
$variant_table = 'mview_ndg_'.$partition.'_variants';
nd_genotypes_create_mview_ndg_variants($variant_table);
print "Created $variant_table.\n";
$germplasm_table = 'mview_ndg_germplasm_genotyped';
nd_genotypes_create_mview_ndg_germplasm_genotyped($germplasm_table);
print "Created $germplasm_table.\n";
