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
    $number_of_markers = count($dataset['data']);

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
    $number_of_markers = count($dataset['data']);

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
    $number_of_markers = count($dataset['data']);

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
    $number_of_markers = count($dataset['data']);

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
    //    Database seeder makes markers between 100-600 on a single chromosome.
    $filter_criteria['seq_range'] = [
      'start' => [
        'backbone' => $dataset['chromosome']['record']->name,
        'pos' => 100],
      'end' => [
        'backbone' => $dataset['chromosome']['record']->name,
        'pos' => 200],
    ];
    $filter_expected['seq_range'] = [];
    foreach ($dataset['data'] as $k => $e) {
      if (($e['position']['start'] >= 100) && ($e['position']['start'] < 200)) {
        $filter_expected['seq_range'][$k] = $e;
      }
    }

    // -- Add variant subset.
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
    $key = array_rand($dataset['data']);
    $value = $dataset['data'][$key];
    $filter_expected['marker_type'][$key] = $value;
    $filter_criteria['marker_type'] = $value['marker_type']['type'];

    // -- Project.
    //    There is a single project.
    //    Choose a random variant.
    $key = array_rand($dataset['data']);
    $value = $dataset['data'][$key];
    $filter_expected['project_id'][$key] = $value;
    $filter_criteria['project_id'] = $value['genotypes'][0]['project']->project_id;

    //print "EXPECTED:" . print_r($filter_expected, TRUE);
    //print "CRITERIA:" . print_r($filter_criteria, TRUE);

    // Now loop through each filter criteria and apply it on it's own.
    foreach ($filter_criteria as $criteria_key => $criterion) {

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

      // -- Now finally, use the function to determine the query.
      $success = nd_genotypes_retrive_matrix_postgresql($vars);
      $this->assertNotFalse($success);

      // -- We expect the exact # of variants since it's less than 1 page.
      $filtered_num_markers = sizeof($filter_expected[$criteria_key]);
      $this->assertCount($filtered_num_markers, $vars['variants'],
        "There was not the correct expected number of markers for " . strtoupper($criteria_key));
      // -- Next check all the variants and genotypes are there.
      $this->check_full_matrix($filter_expected[$criteria_key], $vars, strtoupper($criteria_key));

    }
  }

  /**
   * HELPER: Check full matrix.
   *
   * This helper function will check that all the expected variants
   * and genotype calls are available in the results.
   */
   public function check_full_matrix($expected, $results, $helpful_label) {

     // -- Check all the variants are there. (key: `chr:start_variantid`)
     foreach ($expected as $record_details) {
       $parts = [
         'chr_name' => $record_details['chromosome']['record']->name,
         'start' => $record_details['position']['start'],
         'variant_id' => $record_details['variant']['variant_id'],
       ];
       $key_to_check = $parts['chr_name'].':'.$parts['start'].'_'.$parts['variant_id'];
       $this->assertArrayHasKey($key_to_check, $results['variants'],
         "The expected variant was not in the results for $helpful_label.");

       // -- Check that the genotypes are as expected.
       foreach ($record_details['genotypes'] as $genotype_details) {
         $expected_call = $genotype_details['genotype_call'];
         $allele = $genotype_details['allele'];
         $variant_id = $expected_call['variant_id'];
         $germplasm_id = $genotype_details['germplasm']->stock_id;

         $this->assertArrayHasKey($variant_id, $results['data'],
           "The variant is not in the results for $helpful_label.");
         $this->assertArrayHasKey($germplasm_id, $results['data'][$variant_id],
           "The germplasm is not present for this variant ($helpful_label).");
         $this->assertContains(
           $allele, $results['data'][$variant_id][$germplasm_id],
           "The genotype was not present for this germplasm-variant combo ($helpful_label).");
       }
     }
   }
}
