 # Release Checklist
 
 * Check for critical problems
 * Update translations
   * make sure english catkeys in polyglot are up to date
   * import completed translations into repository
 * Documentation
   * Update README, changelog and screenshot
 * Release
   * Create release branch
   * Bump version
   * Create release from the release branch
   * Review haikuports recipe
     * Change it to point at the new release tag
     * Make a PR to haikuports with the new recipe
 
