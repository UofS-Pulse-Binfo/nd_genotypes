<?php
namespace Tests\genotype_matrix;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use  Tests\DatabaseSeeders\GenotypeDatasetSeeder;

class matrixPagedResultsTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Test the Genotype Matrix with germplasm but no other filter criteria.
   *
   * @group genotype-matrix
   */
  public function testUnfilteredQuery() {
    $current_page_limit = 100;

    // Generate a dataset 1/2 the page limit.
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
    $this->assertCount($number_of_markers, $vars['variants']);
    // -- Check all the variants are there. (key: `chr:start_variantid`)
    foreach ($dataset['data'] as $record_details) {
      $parts = [
        'chr_name' => $record_details['chromosome']['record']->name,
        'start' => $record_details['position']['start'],
        'variant_id' => $record_details['variant']['variant_id'],
      ];
      $key_to_check = $parts['chr_name'].':'.$parts['start'].'_'.$parts['variant_id'];
      $this->assertArrayHasKey($key_to_check, $vars['variants'],
        "The expected variant was not in the results.");

      // -- Check that the genotypes are as expected.
      foreach ($record_details['genotypes'] as $genotype_details) {
        $expected_call = $genotype_details['genotype_call'];
        $allele = $genotype_details['allele'];
        $variant_id = $expected_call['variant_id'];
        $germplasm_id = $genotype_details['germplasm']->stock_id;

        $this->assertArrayHasKey($variant_id, $vars['data'],
          "The variant is not in the results.");
        $this->assertArrayHasKey($germplasm_id, $vars['data'][$variant_id],
          "The germplasm is not present for this variant.");
        $this->assertContains(
          $allele, $vars['data'][$variant_id][$germplasm_id],
          "The genotype was not present for this germplasm-variant combo.");
      }
    }

    // Generate a dataset larger then the page limit.
    $seeder = new GenotypeDatasetSeeder();
    $dataset = $seeder->up($current_page_limit+5, TRUE);
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
  }
}
