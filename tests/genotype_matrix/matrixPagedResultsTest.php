<?php
namespace Tests\genotype_matrix;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use  Tests\DatabaseSeeders\GenotypeDatasetSeeder;

class matrixPagedResultsTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  public function setUp() {

    // Create tables.
    $partition = 'tripalus';
    $calls_table = 'mview_ndg_'.$partition.'_calls';
    nd_genotypes_create_mview_ndg_calls($calls_table);
    $variant_table = 'mview_ndg_'.$partition.'_variants';
    nd_genotypes_create_mview_ndg_variants($variant_table);
    $germplasm_table = 'mview_ndg_germplasm_genotyped';
    nd_genotypes_create_mview_ndg_germplasm_genotyped($germplasm_table);
  }
  /**
   * Test the Genotype Matrix with germplasm but no other filter criteria.
   *
   * @group genotype-matrix
   */
  public function testUnfilteredQuery() {
    $current_page_limit = 100;
    $small_set = 5;

    // Generate a small dataset.
    $seeder = new GenotypeDatasetSeeder();
    $dataset = $seeder->up($small_set, TRUE);
    $number_of_variants = count($dataset['data']);

    // -- Germplasm.
    $germplasm_ids = [];
    $germplasm = [];
    foreach ($dataset['germplasm'] as $key => $stock) {
      $germplasm_ids[$key] = $stock->stock_id;
      $germplasm[$key] = [
        'object' => $stock,
        'name' => $stock->name,
        'class' => $stock->name,
      ];
    }

    // -- Arguements for query.
    $vars = [
      'genus' => 'Tripalus',
      'partition' => 'Tripalus',
      'q' => [],
      'query_args' => [],
      'germplasm' => $germplasm,
      'defaults' => [],
      'header' => [],
      'template_row' => [],
      'variants' => [],
      'data' => [],
    ];
    $vars['q']['germplasm'] = $germplasm_ids;
    $vars['q']['page'] = 1;
    $vars['query_args']['germplasm_id'] = $germplasm_ids;
    $vars['query_args']['page'] = $vars['q']['page'];

    // -- Now finally, use the function to determine the query.
    $success = nd_genotypes_retrive_matrix_postgresql($vars);
    $this->assertNotFalse($success);

    // -- We expect the exact # of variants since it's less than 1 page.
    $this->assertCount($small_set, $vars['variants']);
    // -- Next check all the variants and genotypes are there.
    $this->check_full_matrix($dataset['data'], $vars, 'FULL SET');


    // Generate a dataset larger then the page limit and check the first page.
    $seeder = new GenotypeDatasetSeeder();
    $dataset = $seeder->up($current_page_limit + $small_set, TRUE);
    $number_of_variants = count($dataset['data']);

    // -- Germplasm.
    $germplasm_ids = [];
    $germplasm = [];
    foreach ($dataset['germplasm'] as $key => $stock) {
      $germplasm_ids[$key] = $stock->stock_id;
      $germplasm[$key] = [
        'object' => $stock,
        'name' => $stock->name,
        'class' => $stock->name,
      ];
    }

    // -- Arguements for query.
    $vars = [
      'genus' => 'Tripalus',
      'partition' => 'Tripalus',
      'q' => [],
      'query_args' => [],
      'germplasm' => $germplasm,
      'defaults' => [],
      'header' => [],
      'template_row' => [],
      'variants' => [],
      'data' => [],
    ];
    $vars['q']['germplasm'] = $germplasm_ids;
    $vars['q']['page'] = 1;
    $vars['query_args']['germplasm_id'] = $germplasm_ids;
    $vars['query_args']['page'] = $vars['q']['page'];

    // -- Now finally, use the function to determine the query.
    $success = nd_genotypes_retrive_matrix_postgresql($vars);
    $this->assertNotFalse($success);

    // -- We expect the page limit since it's greater then one page of results.
    $this->assertCount($current_page_limit, $vars['variants']);

    // Generate a dataset larger then the page limit and check the first page.
    $seeder = new GenotypeDatasetSeeder();
    $dataset = $seeder->up($current_page_limit + $small_set, TRUE);
    $number_of_variants = count($dataset['data']);

    // -- Germplasm.
    $germplasm_ids = [];
    $germplasm = [];
    foreach ($dataset['germplasm'] as $key => $stock) {
      $germplasm_ids[$key] = $stock->stock_id;
      $germplasm[$key] = [
        'object' => $stock,
        'name' => $stock->name,
        'class' => $stock->name,
      ];
    }

    // -- Arguements for query.
    $vars = [
      'genus' => 'Tripalus',
      'partition' => 'Tripalus',
      'q' => [],
      'query_args' => [],
      'germplasm' => $germplasm,
      'defaults' => [],
      'header' => [],
      'template_row' => [],
      'variants' => [],
      'data' => [],
    ];
    $vars['q']['germplasm'] = $germplasm_ids;
    $vars['q']['page'] = 2;
    $vars['query_args']['germplasm_id'] = $germplasm_ids;
    $vars['query_args']['page'] = $vars['q']['page'];

    // -- Now finally, use the function to determine the query.
    $success = nd_genotypes_retrive_matrix_postgresql($vars);
    $this->assertNotFalse($success);

    // -- We expect the extra results since it's the second/last page.
    $this->assertCount($small_set, $vars['variants']);
  }

  /**
   * Test the Genotype Matrix with germplasm but no other filter criteria.
   *
   * @group genotype-matrix
   */
  public function testFilteredQueries() {
    $current_page_limit = 100;
    $small_set = 5;

    // Generate a medium dataset.
    $seeder = new GenotypeDatasetSeeder();
    $dataset = $seeder->up($current_page_limit/2, TRUE);
    $number_of_variants = count($dataset['data']);

    // -- Germplasm.
    $germplasm_ids = [];
    $germplasm = [];
    foreach ($dataset['germplasm'] as $key => $stock) {
      $germplasm_ids[$key] = $stock->stock_id;
      $germplasm[$key] = [
        'object' => $stock,
        'name' => $stock->name,
        'class' => $stock->name,
      ];
    }

    // Determine our full set of filter criteria.
    $filter_criteria = [];

    // -- Add a sequence range.
    //    Database seeder makes variants between 100-600 on a single chromosome.
    $filter_criteria['seq_range'] = [
      'start' => [
        'backbone' => $dataset['chromosome']['record']->name,
        'pos' => 100],
      'end' => [
        'backbone' => $dataset['chromosome']['record']->name,
        'pos' => 400],
    ];
    $filter_expected['seq_range'] = [];
    foreach ($dataset['data'] as $k => $e) {
      if (($e['position']['start'] >= 100) && ($e['position']['start'] < 400)) {
        $filter_expected['seq_range'][$k] = $e;
      }
    }

    // -- Add variant subset.
    //    Make sure this is a subset of the sequence range filter
    $filter_expected['variant_name'] = [];
    $filter_criteria['variant_name'] = [];
    $i = 0;
    foreach ($dataset['data'] as $k => $e) {
      $i++;
      if ($i <= 10) {
        $filter_criteria['variant_name'][] = $e['variant']['record']->name;
        $filter_expected['variant_name'][$k] = $e;
      }
      // if we are over 10 then we just want to check for any duplicates
      // to make sure they are included.
      else {
        if (in_array($e['variant']['record']->name, $filter_criteria['variant_name'])) {
          $filter_expected['variant_name'][$k] = $e;
        }
      }
    }

    // -- Variant Type.
    //    All variants created by the seeder are of type SNP.
    $filter_expected['variant_type'] = $dataset['data'];
    $filter_criteria['variant_type'] = 'SNP';

    // -- Marker Type.
    //    Each marker is of it's own type.
    //    Choose a random variant.
    $key = array_rand($filter_expected['variant_name']);
    $value = $dataset['data'][$key];
    $filter_expected['marker_type'][$key] = $value;
    $filter_criteria['marker_type'] = $value['marker_type']['type'];

    // -- Project.
    //    There is a single project.
    //    Use the same variant as for marker type.
    $value = $dataset['data'][$key];
    $filter_expected['project_id'][$key] = $value;
    $filter_criteria['project_id'] = $value['genotypes'][0]['project']->project_id;

    // Now loop through each filter criteria combination (order doesn't matter).
    // Hard coding this b/c I didn't feel like recursively determining it.
    $combos = [
      'SR' => ['seq_range'],
      'SRN' => ['seq_range', 'variant_name'],
      'SRNT' => ['seq_range', 'variant_name', 'variant_type'],
      'SRNTM' => ['seq_range', 'variant_name', 'variant_type', 'marker_type'],
      'SRNTMP' => ['seq_range', 'variant_name', 'variant_type', 'marker_type', 'project_id'],
      'N' => ['variant_name'],
      'NT' => ['variant_name', 'variant_type'],
      'NTM' => ['variant_name', 'variant_type', 'marker_type'],
      'NTMP' => ['variant_name', 'variant_type', 'marker_type', 'project_id'],
      'M' => ['marker_type'],
      'MP' => ['marker_type', 'project_id'],
      'P' => ['project_id'],
    ];
    foreach ($combos as $combo_label => $criteria_combo) {

      // -- Arguements for query.
      $vars = [
        'genus' => 'Tripalus',
        'partition' => 'Tripalus',
        'q' => [],
        'query_args' => [],
        'germplasm' => $germplasm,
        'defaults' => [],
        'header' => [],
        'template_row' => [],
        'variants' => [],
        'data' => [],
      ];
      $vars['q']['germplasm'] = $germplasm_ids;
      $vars['q']['page'] = 1;
      $vars['query_args']['germplasm_id'] = $germplasm_ids;
      $vars['query_args']['page'] = $vars['q']['page'];

      // ::: Add Criteria to Arguements
      $expected_variants = [];
      foreach ($criteria_combo as $criteria_key) {
        $criterion = $filter_criteria[$criteria_key];

        if ($criteria_key == 'seq_range') {
          $vars['q'][$criteria_key] = $criterion;
          $vars['query_args']['start_backbone'] = $criterion['start']['backbone'];
          $vars['query_args']['start_pos'] = $criterion['start']['pos'];
          $vars['query_args']['end_backbone'] = $criterion['end']['backbone'];
          $vars['query_args']['end_pos'] = $criterion['end']['pos'];
        }
        else {
          $vars['q'][$criteria_key] = $criterion;
          $vars['query_args'][$criteria_key] = $criterion;
        }

        // ::: Determine the expected variants for the current combo.
        //     For combos with more then one filter, this will be the maximum
        //     number or variants rather then the exact number.
        $expected_variants = array_merge($expected_variants, $filter_expected[$criteria_key]);
      }

      // -- Now finally, use the function to determine the query.
      $success = nd_genotypes_retrive_matrix_postgresql($vars);
      $this->assertNotFalse($success);

      // -- We expect the exact # of variants since it's less than 1 page.
      $filtered_num_variants = sizeof($expected_variants);
      if (sizeof($criteria_combo) == 1) {
        $this->assertCount($filtered_num_variants, $vars['variants'],
          "There was not the correct expected number of variants for " . $combo_label);
      }
      else {
        $this->assertLessThanOrEqual($filtered_num_variants, sizeof($vars['variants']),
          "There was not the correct expected number of variants for " . $combo_label);
      }
      // -- Next check all the variants and genotypes are there.
      $this->check_full_matrix($expected_variants, $vars, $combo_label);

    }
  }

  /**
   * HELPER: Check full matrix.
   *
   * This helper function will check that all the expected variants
   * and genotype calls are available in the results.
   */
   public function check_full_matrix($expected, $results, $helpful_label) {

     $mapping = [];
     foreach ($expected as $expected_key => $expected_details) {
       $parts = [
         'chr_name' => $expected_details['chromosome']['record']->name,
         'start' => $expected_details['position']['start'],
         'variant_id' => $expected_details['variant']['variant_id'],
       ];
       $result_key = $parts['chr_name'].':'.$parts['start'].'_'.$parts['variant_id'];

       $mapping[$expected_key] = $result_key;
     }

     //print "EXPECTED:".print_r($expected, TRUE);
     //print "RESULT:".print_r($results, TRUE);
     //print "MAPPING:".print_r($mapping, TRUE);

     // -- Check all the variants that are there are expected to be there.
     // (result_key: `chr:start_variantid`)
     foreach ($results['variants'] as $result_key => $var_details) {

       $expected_key = array_search($result_key, $mapping);
       $this->assertNotFalse($expected_key,
         "The result variant was not in the expected list for $helpful_label.");

       // -- Check that the genotypes are as expected.
       $record_details = $expected[$expected_key];
       foreach ($record_details['genotypes'] as $genotype_details) {
         $expected_call = $genotype_details['genotype_call'];
         $allele = $genotype_details['allele'];
         $variant_id = $expected_call['variant_id'];
         $germplasm_id = $genotype_details['germplasm']->stock_id;

         $this->assertArrayHasKey($variant_id, $results['data'],
           "The variant is not in the results data for $helpful_label.");
         $this->assertArrayHasKey($germplasm_id, $results['data'][$variant_id],
           "The germplasm is not present for this variant ($helpful_label).");
         $this->assertContains(
           $allele, $results['data'][$variant_id][$germplasm_id],
           "The genotype was not present for this germplasm-variant combo ($helpful_label).");
       }
     }
   }
}
