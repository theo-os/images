const std = @import("std");

pub fn build(b: *std.build.Builder) void {
    const mode = b.standardReleaseOptions();

    const main = b.addExecutable("theos-init", "init.zig");
    main.setBuildMode(mode);
    //main.linkLibC();
    main.install();

    const run_cmd = main.run();
    run_cmd.step.dependOn(b.getInstallStep());

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
