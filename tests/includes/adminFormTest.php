<?php
namespace Tests;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use  Tests\DatabaseSeeders\GenotypeDatasetSeeder;

class adminFormTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Tests nd_genotypes_admin_landing_page().
   *
   * @group admin
   * @group form
   */
  public function testAdminLandingPage() {
    include_once('includes/nd_genotypes.admin.inc');

    // Generate a small dataset.
    $seeder = new GenotypeDatasetSeeder();
    $dataset = $seeder->up(2, TRUE);
    $number_of_variants = count($dataset['data']);
    nd_genotypes_summarize_mviews();

    // Now that there is data, test the landing page.
    $output = nd_genotypes_admin_landing_page();
    $this->assertStringContainsString(
      'Tripalus',
      $output,
      "The recently added partition was not listed on the landing.");
  }

  /**
   * Tests nd_genotypes_admin_settings()
   *
   * @group admin
   * @group form
   */
  public function testAdminSettingsForm() {
    $form_state = [];
    $form = nd_genotypes_admin_settings([], $form_state);
    $this->assertIsArray($form, "Expect form to be an array.");
    $this->assertNotEmpty($form, "Expect the form to contain elements.");
  }

  /**
   * Tests nd_genotypes_admin_settings_submit()
   *
   * @group admin
   * @group form
   */
  public function testAdminSettingsFormSubmit() {
    $form_state = [ 'values' => []];
    $form_state['values']['storage_method'] = uniqid();

    // First check the SNP Colours are saved.
    $form_state['triggering_element']['#value'] = 'Save SNP Colours';
    $form_state['values']['SNPs'] = [1,2,3,4];
    nd_genotypes_admin_settings_submit([], $form_state);
    $retrieved_colours = variable_get('nd_genotypes_SNP_colours', NULL);
    $this->assertNotNull($retrieved_colours, "Colours were not saved.");
    $unserialized = unserialize($retrieved_colours);
    $this->assertIsArray($unserialized, "Unable to unserialize the colours.");
    $this->assertEquals([1,2,3,4], $unserialized,
      "Expect the colours variable to contain what we set.");

    // First check the Generic colourset is saved.
    $form_state['triggering_element']['#value'] = 'Save Generic Allele Colourset';
    $form_state['values']['allele_colors'] = '1  2  3  4';
    nd_genotypes_admin_settings_submit([], $form_state);
    $retrieved_colours = variable_get('nd_genotypes_allele_colours', NULL);
    $this->assertNotNull($retrieved_colours, "Generic Colours were not saved.");
    $unserialized = unserialize($retrieved_colours);
    $this->assertIsArray($unserialized, "Unable to unserialize the generic colours.");
    $this->assertEquals([1,2,3,4], $unserialized,
      "Expect the generic colours variable to contain what we set.");
  }

  /**
   * Tests nd_genotypes_admin_settings_submit()
   *
   * @group admin
   * @group form
   */
  public function testAdminSettingsCVTERMsFormSubmit() {
    $form_state = [ 'values' => []];
    $form_state['values']['markerprop'] = 1;
    $form_state['values']['variantprop'] = 2;
    $form_state['values']['reltypes'] = serialize([1 => a, 2 => b, 3 => c]);
    $form_state['values']['marker_reltype'] = 3;
    $form_state['values']['marker_relposition'] = 'fred';
    $form_state['values']['stock_reltype'] = 4;
    $form_state['values']['stock_relposition'] = 'sarah';
    $form_state['values']['follow_project_rels'] = TRUE;
    $form_state['values']['subproject_type_id'] = 5;
    $form_state['values']['subproject_position'] = 'chi';

    nd_genotypes_admin_settings_cvterms_submit($form, $form_state);

    // Now check each of the variables was set properly.
    $var = variable_get('nd_genotypes_markerprop_type_id', NULL);
    $this->assertNotNull($var, 'nd_genotypes_markerprop_type_id');
    $this->assertEquals($var, 1, 'nd_genotypes_markerprop_type_id');
    $var = variable_get('nd_genotypes_variantprop_type_id', NULL);
    $this->assertNotNull($var, 'nd_genotypes_variantprop_type_id');
    $this->assertEquals($var, 2, 'nd_genotypes_variantprop_type_id');
    $var = variable_get('nd_genotypes_rel_type_id', NULL);
    $this->assertNotNull($var, 'nd_genotypes_rel_type_id');
    $this->assertEquals($var, 3, 'nd_genotypes_rel_type_id');
    $var = variable_get('nd_genotypes_rel_type_name', NULL);
    $this->assertNotNull($var, 'nd_genotypes_rel_type_name');
    $this->assertEquals($var, 'c', 'nd_genotypes_rel_type_name');
    $var = variable_get('nd_genotypes_rel_position', NULL);
    $this->assertNotNull($var, 'nd_genotypes_rel_position');
    $this->assertEquals($var, 'fred', 'nd_genotypes_rel_position');
    $var = variable_get('nd_genotypes_stock_rel_type_id', NULL);
    $this->assertNotNull($var, 'nd_genotypes_stock_rel_type_id');
    $this->assertEquals($var, 4, 'nd_genotypes_stock_rel_type_id');
    $var = variable_get('nd_genotypes_stock_rel_position', NULL);
    $this->assertNotNull($var, 'nd_genotypes_stock_rel_position');
    $this->assertEquals($var, 'sarah', 'nd_genotypes_stock_rel_position');
    $var = variable_get('ndg_follow_project_rels', NULL);
    $this->assertNotNull($var, 'ndg_follow_project_rels');
    $this->assertEquals($var, TRUE, 'ndg_follow_project_rels');
    $var = variable_get('nd_genotypes_subproject_rel_type_id', NULL);
    $this->assertNotNull($var, 'nd_genotypes_subproject_rel_type_id');
    $this->assertEquals($var, 5, 'nd_genotypes_subproject_rel_type_id');
    $var = variable_get('nd_genotypes_subproject_rel_position', NULL);
    $this->assertNotNull($var, 'nd_genotypes_subproject_rel_position');
    $this->assertEquals($var, 'chi', 'nd_genotypes_subproject_rel_position');

  }

  /**
   * Tests nd_genotypes_admin_sync_mviews()
   *
   * @group admin
   * @group form
   */
  public function testAdminSettingsSynchMviewsFormt() {
    $form_state = [];
    $form = nd_genotypes_admin_sync_mviews([], $form_state);
    $this->assertIsArray($form, "Expect form to be an array.");
    $this->assertNotEmpty($form, "Expect the form to contain elements.");
  }
}
