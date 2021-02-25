describe('native module', () => {
  it('is healthy', () => {
    expect(require('..').native.ready).toBe(true);
  });

  it('can be required again, but does nothing', () => {
    expect(require('..').native.ready).toBe(true);
  });

  it('works with jest module hacks (but does not reload)', () => {
    jest.isolateModules(() => {
      expect(require('..').native.ready).toBe(true);
    });
  });
});
