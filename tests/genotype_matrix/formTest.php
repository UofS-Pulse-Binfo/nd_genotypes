<?php
namespace Tests\genotype_matrix;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;

class formTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Test the genotype matrix form displayed at chado/genotype/[genus]
   * for the purpose of filtering variants/genotypes.
   *
   * @group genotype-matrix
   * @group form
   */
  public function testGenotypeMatrixForm() {

    $form = [];
    $form_state = [];
    $form_state['build_info']['args'][0]['genus'] = 'Tripalus';
    $form_state['build_info']['args'][0]['partition'] = 'tripalus';

    // First check the form builds without error when there are no defaults.
    $form_state['build_info']['args'][0]['defaults'] = [];
    $form = nd_genotypes_matrix_filter_form($form, $form_state);
    $this->assertNotEmpty($form, 'The form didn\'t build correctly.');
    $this->assertCount(4, $form,
      "There are not the expected number of form sections.");
    $this->assertCount(2, element_children($form['s1']),
      "There are not the expected number of form elements in germplasm section.");
    $this->assertCount(1, element_children($form['s2']),
      "There are not the expected number of form elements in range section.");
    $this->assertCount(4, element_children($form['s3']),
      "There are not the expected number of form elements in advanced section.");
  }

  /**
   * Test the genotype matrix form displayed at chado/genotype/[genus]
   * for the purpose of filtering variants/genotypes.
   *
   * @group genotype-matrix
   * @group form
   */
  public function testGenotypeMatrixFormValidate() {

    $form = [];
    $form_state = [];
    $form_state['build_info']['args'][0]['genus'] = 'Tripalus';
    $form_state['build_info']['args'][0]['partition'] = 'tripalus';
    $form_state['values'] = [];

    // -- Genome Range
    // If the postition is supplied then the chromosome should be to.
    $form_state['values']['seq_range']['start']['pos'] = 5;
    $form_state['values']['seq_range']['start']['backbone'] = NULL;
    $errors = nd_genotypes_matrix_filter_form_validate($form, $form_state);
    $this->assertArrayHasKey('start-backbone-missing', $errors,
      "The backbone is required if the position is supplied: start.");

    $form_state['values']['seq_range']['end']['pos'] = 5;
    $form_state['values']['seq_range']['end']['backbone'] = NULL;
    $errors = nd_genotypes_matrix_filter_form_validate($form, $form_state);
    $this->assertArrayHasKey('end-backbone-missing', $errors,
      "The backbone is required if the position is supplied: end.");

    // The position should be numeric.
    $form_state['values']['seq_range']['start']['pos'] = 'NOT NUMERIC';
    $form_state['values']['seq_range']['start']['backbone'] = 'fake chromosome';
    $errors = nd_genotypes_matrix_filter_form_validate($form, $form_state);
    $this->assertArrayHasKey('start-not-numeric', $errors,
      "The position must be numeric: start.");

    $form_state['values']['seq_range']['end']['pos'] = 'NOT NUMERIC';
    $form_state['values']['seq_range']['end']['backbone'] = 'fake chromosome';
    $errors = nd_genotypes_matrix_filter_form_validate($form, $form_state);
    $this->assertArrayHasKey('end-not-numeric', $errors,
      "The position must be numeric: end.");

    // The start should be smaller than the end.
    // -- Same backbone.
    $form_state['values']['seq_range']['start']['pos'] = 10;
    $form_state['values']['seq_range']['start']['backbone'] = 'fake chromosome';
    $form_state['values']['seq_range']['end']['pos'] = 2;
    $form_state['values']['seq_range']['end']['backbone'] = 'fake chromosome';
    $errors = nd_genotypes_matrix_filter_form_validate($form, $form_state);
    $this->assertArrayHasKey('end<start', $errors,
      "The end must be greater then the start.");
    // -- Different backbone.
    $form_state['values']['seq_range']['start']['pos'] = 10;
    $form_state['values']['seq_range']['start']['backbone'] = 'Chr2';
    $form_state['values']['seq_range']['end']['pos'] = 2;
    $form_state['values']['seq_range']['end']['backbone'] = 'Chr1';
    $errors = nd_genotypes_matrix_filter_form_validate($form, $form_state);
    $this->assertArrayHasKey('end<start', $errors,
      "The end must be greater then the start.");

    // -- Variant Names.
    // Warning: if there are commas or tabs, perhaps you messed up the format?
    $form_state['values']['variant_name'] = '1,2,3,4';
    $errors = nd_genotypes_matrix_filter_form_validate($form, $form_state);
    $this->assertArrayHasKey('variant-name-comma', $errors,
      "The variant name field should not contain comma's. This is a warning in the form.");

    $form_state['values']['variant_name'] = "quis\tme\tyour\tmom";
    $errors = nd_genotypes_matrix_filter_form_validate($form, $form_state);
    $this->assertArrayHasKey('variant-name-tab', $errors,
      "The variant name field should not contain tabs. This is a warning in the form.");

    // -- Check project exists.
    $form_state['values']['project_name'] = 'FAKE PROJECT'.uniqid();
    $errors = nd_genotypes_matrix_filter_form_validate($form, $form_state);
    $this->assertArrayHasKey('project-not-exist', $errors,
      "The project must exist.");
  }

  /**
   * Test the genotype matrix form displayed at chado/genotype/[genus]
   * for the purpose of filtering variants/genotypes.
   *
   * @group genotype-matrix
   * @group form
   */
  public function testGenotypeMatrixFormSubmit() {

    $form = [];
    $form_state = [];
    $form_state['build_info']['args'][0]['genus'] = 'Tripalus';
    $form_state['build_info']['args'][0]['partition'] = 'tripalus';
    $form_state['phpunit'] = TRUE; // Stop redirect.

    // -- First just test it with a version of all filter criteria.
    $form_state['values'] = [];
    $form_state['values']['multiple_stocks']['existing'] = [
      ['stock_id' => 1],
      ['stock_id' => 2],
    ];
    $form_state['values']['polymorphic']['first'] = 1;
    $form_state['values']['polymorphic']['second'] = 2;
    $form_state['values']['seq_range']['start']['pos'] = 5;
    $form_state['values']['seq_range']['start']['backbone'] = 'Chr1';
    $form_state['values']['seq_range']['end']['pos'] = 10;
    $form_state['values']['seq_range']['end']['backbone'] = 'Chr2';
    $form_state['values']['variant_name'] = "fakep0\nfakep1\nfakep2";
    $form_state['values']['project_name'] = 'FAKE PROJECT'.uniqid();
    $form_state['values']['marker_type'] = 'Exome Capture';
    $form_state['values']['variant_type'] = 'SNP';

    // First check the form builds without error when there are no defaults.
    $form_state['build_info']['args'][0]['defaults'] = [];
    $form = nd_genotypes_matrix_filter_form($form, $form_state);
    $this->assertNotEmpty($form, 'The form didn\'t build correctly.');
    $redirect = nd_genotypes_matrix_filter_form_submit($form, $form_state);
    $this->assertNotEmpty($redirect, 'The form submit didn\'t execute correctly.');

    // Check the redirect is correct.
    $redirect_query = $redirect[1]['query'];
    $expected = [
      'germplasm' => [1,2],
      'seq_range' => [
        'start' => [ 'pos' => 5, 'backbone' => 'Chr1' ],
        'end' => [ 'pos' => 10, 'backbone' => 'Chr2' ],
      ],
      'sort' => 'variant_name',
      'variant_name' => [ 'fakep0', 'fakep1', 'fakep2' ],
      'polymorphic' => [ 'first' => 1, 'second' => 2 ],
      'project_name' => $form_state['values']['project_name'],
      'marker_type' => 'Exome Capture',
      'variant_type' => 'SNP',
      'page' => 1,
    ];
    $this->assertEquals($expected, $redirect_query,
      "The query portion of the redirect does not match what we expected.");

    // -- Specifically, test the creation of saved variant lists.
    $variant_names = '';
    for ($i=0; $i<=30; $i++) {
      $variant_names .= "fakep$i\n";
    }
    $form_state['values']['variant_name'] = $variant_names;
    $form = nd_genotypes_matrix_filter_form($form, $form_state);
    $this->assertNotEmpty($form, 'The form didn\'t build correctly.');
    $redirect = nd_genotypes_matrix_filter_form_submit($form, $form_state);
    $this->assertNotEmpty($redirect, 'The form submit didn\'t execute correctly.');
    $redirect_query = $redirect[1]['query'];

    // Check that the variant_names have been switched to a list.
    $this->assertArrayNotHasKey('variant_name', $redirect_query,
      "The presence of variant_names implies the filter was not made into a saved list.");
    $this->assertArrayHasKey('variant_name_list', $redirect_query,
      "The saved list ID should be in the redirect query.");
    $this->assertIsNumeric($redirect_query['variant_name_list'],
      "This saved list ID should be numeric.");

    // -- Now test that if we give the submit function a saved list,
    //    it keeps it.
    $form_state['values']['variant_name_list'] = $redirect_query['variant_name_list'];
    unset($form_state['values']['variant_name']);

    $form = nd_genotypes_matrix_filter_form($form, $form_state);
    $this->assertNotEmpty($form, 'The form didn\'t build correctly.');
    $redirect = nd_genotypes_matrix_filter_form_submit($form, $form_state);
    $this->assertNotEmpty($redirect, 'The form submit didn\'t execute correctly.');
    $redirect_query = $redirect[1]['query'];

    // Check that the variant_names have been switched to a list.
    $this->assertArrayNotHasKey('variant_name', $redirect_query,
      "The presence of variant_names implies the filter was not made into a saved list.");
    $this->assertArrayHasKey('variant_name_list', $redirect_query,
      "The saved list ID should be in the redirect query.");
    $this->assertIsNumeric($redirect_query['variant_name_list'],
      "This saved list ID should be numeric.");

    // -- Now is an easy time to check that we can process query paramters
    //    So lets do that too!
    $vars = ['q' => $redirect_query];
    nd_genotypes_process_query_parameters($vars);
    $this->assertArrayHasKey('defaults', $vars,
      "The default values were not processed based on the query paramters.");
    $this->assertIsArray($vars['defaults'],
      "There default values should be an array.");
    $this->assertNotEmpty($vars['defaults'],
      "There are no default values.");
    $this->assertArrayHasKey('query_args', $vars,
      "Query args should be processed into an easier to consume form.");
    $this->assertIsArray($vars['query_args'],
      "Processed query args should be an array.");
    $this->assertNotEmpty($vars['query_args'],
      "Processed query args should not be empty.");
  }
}
