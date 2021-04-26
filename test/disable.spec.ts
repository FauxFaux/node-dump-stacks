import { spawnSync } from 'child_process';

describe('disabling via env', () => {
  it('exports an empty module', () => {
    process.env['DUMP_STACKS_ENABLED'] = 'false';
    expect(require('..').native).toEqual({});
  });

  it('does not dump stacks', () => {
    const child = spawnSync(
      process.argv[0],
      [require.resolve('./child'), '250'],
      {
        stdio: ['ignore', 'inherit', 'pipe'],
        encoding: 'utf-8',
        env: {
          DUMP_STACKS_OBSERVE_MS: '20',
          DUMP_STACKS_CHECK_MS: '20',
          DUMP_STACKS_REPORT_ONCE_MS: '200',
          DUMP_STACKS_ENABLED: 'false',
        },
      },
    );
    if (child.error) {
      throw child.error;
    }
    const prefix = '{"name":"dump-stacks"';
    expect(child.stderr).not.toContain(prefix);
  });
});
