import * as assert from "assert";

import * as vscode from "vscode";
import * as extension from "../extension";

function configTarget(): vscode.ConfigurationTarget {
  return vscode.workspace.workspaceFolders?.length
    ? vscode.ConfigurationTarget.Workspace
    : vscode.ConfigurationTarget.Global;
}

suite("Extension Test Suite", () => {
  suiteSetup(() => {
    const context = {
      subscriptions: [],
      extension: { id: "genmc-vscode" },
    } as any as vscode.ExtensionContext;
    extension.activate(context);
  });

  test("activate registers genmc commands", async () => {
    const commands = await vscode.commands.getCommands(true);
    assert.ok(commands.includes("genmc.runOnActiveFile"));
    assert.ok(commands.includes("genmc.configure"));
  });

  test("genmc.configure opens extension settings", async () => {
    const original = vscode.commands.executeCommand;
    const calls: Array<{ command: string; args: any[] }> = [];
    (vscode.commands as any).executeCommand = (
      command: string,
      ...args: any[]
    ) => {
      calls.push({ command, args });
      if (command === "workbench.action.openSettings") {
        return Promise.resolve(undefined);
      }
      return original(command, ...args);
    };

    try {
      await original("genmc.configure");
    } finally {
      (vscode.commands as any).executeCommand = original;
    }

    assert.ok(
      calls.some(
        (c) =>
          c.command === "workbench.action.openSettings" &&
          typeof c.args[0] === "string" &&
          c.args[0].includes("@ext:genmc-vscode"),
      ),
      `Expected openSettings(@ext:genmc-vscode), got: ${JSON.stringify(calls)}`,
    );
  });

  test("runOnActiveFile shows error when no editor is active", async () => {
    await vscode.commands.executeCommand("workbench.action.closeAllEditors");

    const original = vscode.window.showErrorMessage;
    const seen: string[] = [];
    (vscode.window as any).showErrorMessage = (
      message: string,
      ..._items: any[]
    ) => {
      seen.push(message);
      return Promise.resolve(undefined);
    };

    try {
      await vscode.commands.executeCommand("genmc.runOnActiveFile");
    } finally {
      (vscode.window as any).showErrorMessage = original;
    }

    assert.ok(
      seen.includes("No active editor file to run GenMC on."),
      `Expected "no active editor" error, got: ${JSON.stringify(seen)}`,
    );
  });

  test("runOnActiveFile prompts when genmc.path is not absolute", async () => {
    const cfg = vscode.workspace.getConfiguration("genmc");
    const oldPath = cfg.get<string>("path", "");
    await cfg.update("path", "./genmc", configTarget());

    const doc = await vscode.workspace.openTextDocument({
      content: "int main(){return 0;}\n",
    });
    await vscode.window.showTextDocument(doc, { preview: false });

    const originalError = vscode.window.showErrorMessage;
    (vscode.window as any).showErrorMessage = (
      _message: string,
      ..._items: any[]
    ) => Promise.resolve("Open Settings");

    const originalExec = vscode.commands.executeCommand;
    const calls: Array<{ command: string; args: any[] }> = [];
    (vscode.commands as any).executeCommand = (
      command: string,
      ...args: any[]
    ) => {
      calls.push({ command, args });
      if (command === "workbench.action.openSettings") {
        return Promise.resolve(undefined);
      }
      return originalExec(command, ...args);
    };

    try {
      await originalExec("genmc.runOnActiveFile");
    } finally {
      (vscode.window as any).showErrorMessage = originalError;
      (vscode.commands as any).executeCommand = originalExec;
      await cfg.update("path", oldPath, configTarget());
    }

    assert.ok(
      calls.some(
        (c) =>
          c.command === "workbench.action.openSettings" &&
          c.args[0] === "genmc.path",
      ),
      `Expected openSettings('genmc.path'), got: ${JSON.stringify(calls)}`,
    );
  });

  test("runOnActiveFile shows error when configured binary is missing", async () => {
    const cfg = vscode.workspace.getConfiguration("genmc");
    const oldPath = cfg.get<string>("path", "");
    await cfg.update("path", "/this/does/not/exist/genmc", configTarget());

    const doc = await vscode.workspace.openTextDocument({
      content: "int main(){return 0;}\n",
    });
    await vscode.window.showTextDocument(doc, { preview: false });

    const original = vscode.window.showErrorMessage;
    const seen: string[] = [];
    (vscode.window as any).showErrorMessage = (
      message: string,
      ..._items: any[]
    ) => {
      seen.push(message);
      return Promise.resolve(undefined);
    };

    try {
      await vscode.commands.executeCommand("genmc.runOnActiveFile");
    } finally {
      (vscode.window as any).showErrorMessage = original;
      await cfg.update("path", oldPath, configTarget());
    }

    assert.ok(
      seen.some((m) => m.includes("GenMC executable not found at:")),
      `Expected missing-binary error, got: ${JSON.stringify(seen)}`,
    );
  });
});
