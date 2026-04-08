import * as vscode from "vscode";
import { spawn } from "child_process";
import * as path from "path";
import * as fs from "fs";

export function activate(context: vscode.ExtensionContext) {
  console.log("genmc-vscode activated");

  const output = vscode.window.createOutputChannel("GenMC");
  context.subscriptions.push(output);

  const disposable = vscode.commands.registerCommand(
    "genmc.runOnActiveFile",
    async () => {
      const editor = vscode.window.activeTextEditor;
      if (!editor) {
        vscode.window.showErrorMessage(
          "No active editor file to run GenMC on.",
        );
        return;
      }

      const filePath = editor.document.fileName;

      const cfg = vscode.workspace.getConfiguration("genmc");
      const configuredPath = (cfg.get<string>("path", "") || "").trim();
      const mode = cfg.get<string>("mode", "verify");
      const model = cfg.get<string>("model", "rc11");
      const budget = cfg.get<number>("budget", 1000);
      const scheduleSeed = (cfg.get<string>("scheduleSeed", "") || "").trim();
      const printScheduleSeed = cfg.get<boolean>("printScheduleSeed", false);

      const genmcCmd = configuredPath !== "" ? configuredPath : "genmc";

      if (configuredPath !== "") {
        if (!path.isAbsolute(genmcCmd)) {
          const choice = await vscode.window.showErrorMessage(
            `genmc.path must be an absolute path (got: ${genmcCmd}).`,
            "Open Settings",
          );
          if (choice === "Open Settings") {
            await vscode.commands.executeCommand(
              "workbench.action.openSettings",
              "genmc.path",
            );
          }
          return;
        }

        if (!fs.existsSync(genmcCmd)) {
          const choice = await vscode.window.showErrorMessage(
            `GenMC executable not found at: ${genmcCmd}`,
            "Open Settings",
          );
          if (choice === "Open Settings") {
            await vscode.commands.executeCommand(
              "workbench.action.openSettings",
              "genmc.path",
            );
          }
          return;
        }
      }

      const args: string[] = [];

      args.push(`--${model}`);

      switch (mode) {
        case "verify":
          args.push("--mode=verify");
          break;
        case "random":
          args.push("--mode=random");
          if (Number.isFinite(budget) && budget > 0) {
            args.push(`--budget=${Math.floor(budget)}`);
          }
          if (scheduleSeed !== "") {
            args.push(`--schedule-seed=${scheduleSeed}`);
          }
          if (printScheduleSeed) {
            args.push("--print-schedule-seed");
          }
          break;
        case "estimate":
          args.push("--mode=estimate");
          break;
      }

      args.push("--", filePath);

      output.clear();
      output.show(true);
      output.appendLine(`mode=${mode}`);
      output.appendLine(
        `$ ${genmcCmd} ${args.map((a) => JSON.stringify(a)).join(" ")}`,
      );
      output.appendLine("");

      const proc = spawn(genmcCmd, args, {
        cwd: path.dirname(filePath),
        stdio: ["ignore", "pipe", "pipe"],
      });

      proc.stdout.on("data", (chunk) => output.append(chunk.toString()));
      proc.stderr.on("data", (chunk) => output.append(chunk.toString()));

      proc.on("error", async (err: any) => {
        output.appendLine(`\n[error] ${String(err)}`);

        const isNotFound = err?.code === "ENOENT";
        if (configuredPath === "" && isNotFound) {
          const choice = await vscode.window.showErrorMessage(
            'GenMC is not configured. Set "genmc.path" (or install "genmc" on your PATH).',
            "Open Settings",
          );
          if (choice === "Open Settings") {
            await vscode.commands.executeCommand(
              "workbench.action.openSettings",
              `@ext:${context.extension.id}`,
            );
          }
          return;
        }

        const choice = await vscode.window.showErrorMessage(
          `Failed to start GenMC: ${String(err)}`,
          "Open Settings",
        );
        if (choice === "Open Settings") {
          await vscode.commands.executeCommand(
            "workbench.action.openSettings",
            `@ext:${context.extension.id}`,
          );
        }
      });

      proc.on("close", (code, signal) => {
        output.appendLine(
          `\n[done] exited with code=${code} signal=${signal ?? "none"}`,
        );
        if (code === 0) {
          vscode.window.showInformationMessage("GenMC finished.");
        } else {
          vscode.window.showWarningMessage(
            `GenMC finished with exit code ${code}. See GenMC output.`,
          );
        }
      });
    },
  );

  context.subscriptions.push(disposable);

  const configureDisposable = vscode.commands.registerCommand(
    "genmc.configure",
    async () => {
      await vscode.commands.executeCommand(
        "workbench.action.openSettings",
        `@ext:${context.extension.id}`,
      );
    },
  );

  context.subscriptions.push(configureDisposable);
}

export function deactivate() {}
