describe('native module', () => {
  it('is hello', () => {
    expect(require('..').native.hello()).toBe('world');
  });
});
