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
    $form_state['values']['seq_range']['start']['pos'] = 10;
    $form_state['values']['seq_range']['start']['backbone'] = 'fake chromosome';
    $form_state['values']['seq_range']['end']['pos'] = 2;
    $form_state['values']['seq_range']['end']['backbone'] = 'fake chromosome';
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
}
