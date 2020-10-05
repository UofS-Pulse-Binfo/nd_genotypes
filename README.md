[![Tripal Rating Gold Status](https://tripal.readthedocs.io/en/7.x-3.x/_images/Tripal-Gold.png)](https://tripal.readthedocs.io/en/7.x-3.x/extensions/module_rating.html#Gold)
![Tripal Dependency](https://img.shields.io/badge/tripal-%3E=3.0-brightgreen)
![GitHub release (latest by date including pre-releases)](https://img.shields.io/github/v/release/UofS-Pulse-Binfo/nd_genotypes?include_prereleases)

[![Build Status](https://travis-ci.org/UofS-Pulse-Binfo/nd_genotypes.svg?branch=7.x-3.x)](https://travis-ci.org/UofS-Pulse-Binfo/nd_genotypes)
[![Maintainability](https://api.codeclimate.com/v1/badges/fe04c14638512f7a41f3/maintainability)](https://codeclimate.com/github/UofS-Pulse-Binfo/nd_genotypes/maintainability)
[![Test Coverage](https://api.codeclimate.com/v1/badges/fe04c14638512f7a41f3/test_coverage)](https://codeclimate.com/github/UofS-Pulse-Binfo/nd_genotypes/test_coverage)

[![DOI](https://zenodo.org/badge/44354762.svg)](https://zenodo.org/badge/latestdoi/44354762)

# Natural Diversity Genotypes
This module provides support and visualization of genotypic data stored in a modified GMOD Chado schema. The 3.x branch of this module represents a shift towards support for **large scale genotypic datasets** through backwards compatible improvements to the Chado schema including a new gathering table for genotypes (genotype_call) modelled after the chado phenotype table, optimized queries and well-choosen indexes. For benchmarking demonstrating the efficiency of this module, see our [documentation](https://nd-genotypes.readthedocs.io/en/latest/data_storage/benchmarking.html).

## Dependencies
1. [Tripal 3.x](https://github.com/tripal/tripal) ([Installation Instructions](https://tripal.readthedocs.io/en/latest/user_guide.html))
2. PostgreSQL 9.3 (9.4+ recommended; tested with 11.3)
3. [Tripal D3.js](https://github.com/tripal/tripald3) & [Drupal Libraries API](https://www.drupal.org/project/libraries)
4. [Tripal Download API](https://github.com/tripal/trpdownload_api)


## Installation
This module is installed by cloning it and it's dependencies in `[your drupal site]/sites/all/modules` and enabling it through the Drupal Administrative UI. Specifically, once you have a working Tripal environment:

```
drush pm-download libraries
drush pm-enable libraries -y
cd [drupal root]/sites/all/modules
git clone https://github.com/tripal/tripald3
git clone https://github.com/tripal/trpdownload_api
cd [drupal root]/sites/all/libraries
mkdir d3 && cd d3
wget https://github.com/d3/d3/releases/download/v3.5.17/d3.zip
unzip d3.zip
drush pm-enable trpdownload_api tripald3 -y
cd [drupal root]/sites/all/modules
git clone https://github.com/UofS-Pulse-Binfo/nd_genotypes.git
drush en nd_genotypes -y
```

## Features
- Extensive configuration allowing for flexiblity in ontology terms used, as well as, colours and wording used in visualizations.
- Multiple Tripal 3 Fields which provide flexible, configurable summaries of genotypic data.
- Genotype Matrix search allowing users to extract genotypes for a user-defined set of germplasm.
- Integration of all fields with Tripal 3 web services allowing you to share your genotypic data with other groups.

See [Demonstration](https://nd-genotypes.readthedocs.io/en/latest/contribute.html#manual-testing-demonstration) for how to demonstrate these features on your test site with simulated data.

## Documentation [![Documentation Status](https://readthedocs.org/projects/nd-genotypes/badge/?version=latest)](https://nd-genotypes.readthedocs.io/en/latest/?badge=latest)

Please visit our [online documentation](https://nd-genotypes.readthedocs.io/en/latest/index.html) to learn more about installation, usage and features.

## Funding

This work is supported by Saskatchewan Pulse Growers [grant: BRE1516, BRE0601], Western Grains Research Foundation, Genome Canada [grant: 8302, 16302], Government of Saskatchewan [grant: 20150331], and the University of Saskatchewan.

## Citation [![DOI](https://zenodo.org/badge/44354762.svg)](https://zenodo.org/badge/latestdoi/44354762)

Sanderson, L.A. (2020). ND Genotypes: Large-scale Genotypic data integration with Tripal. Version 3.2. University of Saskatchewan, Pulse Crop Research Group, Saskatoon, SK, Canada. Zenodo, doi:10.5281/zenodo.3731337.
