const std = @import("std");
const linux = std.os.linux;

pub const Service = struct {
    name: []const u8,
    args: [][]const u8,
    background: bool,
};

pub const ServiceConfig = struct {
    mountFileSystems: bool,
    services: []const Service,
};

pub fn main() !void {
    var file = try std.fs.cwd().openFile("boot.json", .{ .read = true });
    defer file.close();

    var servicesBuffer = try std.heap.page_allocator.alloc(u8, std.mem.page_size);
    _ = try file.read(servicesBuffer);

    const services = try std.json.parse(ServiceConfig, &std.json.TokenStream.init(servicesBuffer), std.json.ParseOptions{
        .allow_trailing_data = true,
        .allocator = std.heap.page_allocator,
    });

    if (services.mountFileSystems) {
        std.log.info("mounting rootfs...", .{});
        _ = linux.mount("/", "/", "", linux.MS.REMOUNT, 0);

        std.log.info("mounting procfs...", .{});
        _ = linux.mount("proc", "/proc", "proc", 0, 0);
    }

    std.log.info("starting services...", .{});
    for (services.services) |service| {
        std.log.info("starting service: {s}", .{service.name});
        
        //var buf: [std.fs.MAX_PATH_BYTES]u8 = undefined;
        //const cwd = try std.os.getcwd(&buf);
        const cp = try std.ChildProcess.init(service.args, std.heap.page_allocator);
    
        if (service.background) {
            cp.stdin_behavior = std.ChildProcess.StdIo.Ignore;
        }

        try cp.spawn();
    }

    while (true) {
        std.time.sleep(1000);
    }
}
